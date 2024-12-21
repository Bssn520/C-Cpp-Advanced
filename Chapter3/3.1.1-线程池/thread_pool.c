#include "thread_pool.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 任务类
typedef struct task_s
{
    void *next;
    handler_ptr func;
    void *arg;
} task_t;

// 任务队列类
typedef struct task_queue_s
{
    void *head;
    void **tail;
    atomic_int block;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_spinlock_t lock;
} task_queue_t;

// 线程池类
typedef struct thread_pool_s
{
    task_queue_t *task_queue;
    atomic_int quit; /* 1 为退出；0 为不退出 */
    int thread_count;
    pthread_t *threads;
} thread_pool_t;

// 创建任务队列 使用回滚式编程
static task_queue_t *
__taskqueue_create()
{
    int ret;
    task_queue_t *queue = (task_queue_t *)malloc(sizeof(task_queue_t));
    if (queue) // 如果 queue 创建成功则进行 mutex 初始化
    {
        ret = pthread_mutex_init(&queue->mutex, NULL);
        if (ret == 0) // 如果 mutex 初始化成功，则进行 cond 初始化
        {
            ret = pthread_cond_init(&queue->cond, NULL);
            if (ret == 0) // 如果 cond 初始化成功，则进行spinlock的初始化
            {
                ret = pthread_spin_init(&queue->lock, PTHREAD_PROCESS_PRIVATE);
                if (ret == 0) // 如果 spinlock 初始化成功 则初始化其余成员
                {
                    queue->head = NULL;
                    queue->tail = &queue->head;
                    atomic_init(&queue->block, 1);
                    return queue;
                }
                else
                {
                    pthread_spin_destroy(&queue->lock);
                }
                pthread_cond_destroy(&queue->cond);
            }
            pthread_mutex_destroy(&queue->mutex);
        }
        free(queue);
    }
    return NULL;
}

// 设置队列为非阻塞
static void
__nonblock(task_queue_t *queue)
{

    atomic_store(&queue->block, 0);
    pthread_cond_broadcast(&queue->cond);
}

// 向队列中添加任务
static inline void
__add_task(task_queue_t *queue, void *task)
{
    // 二级指针的妙处在于，此处 task 的参数类型为 void，意味着能够传入不同类型的 task，前提条件为该类型的起始内存为一个用于链接(*next)的指针
    void **link = (void **)task;
    *link = NULL; // 将 task->next 置为NULL

    pthread_spin_lock(&queue->lock);
    *queue->tail /* 等价于 queue->tail->next */ = link; // 将队列中最后一个节点的 next 指向 新节点
    queue->tail = link; // 将队列的尾节点继续设置为NULL
    pthread_spin_unlock(&queue->lock);
    pthread_cond_signal(&queue->cond); // 唤醒线程
}

// 出对一个任务
// 防御式编程 不满足逻辑直接 return
static inline void *
__pop_task(task_queue_t *queue)
{
    pthread_spin_lock(&queue->lock);
    if (queue->head == NULL) // 先判断队列是否为空
    {
        pthread_spin_unlock(&queue->lock);
        return NULL;
    }
    task_t *task;
    task = queue->head;
    // 设置 head 指向被pop节点的下一个节点
    // void **link = (void **)task;
    // queue->head = *link; // *link 即 task->next 指针
    queue->head = task->next;

    // 如果 pop 后队列为空
    if (queue->head == NULL)
    {
        queue->tail = &queue->head; // 1
    }
    pthread_spin_unlock(&queue->lock);

    return task;
}

// 从队列中获取一个任务
static inline void *
__get_task(task_queue_t *queue)
{
    task_t *task;
    // while 循环是为了防止虚假唤醒
    while ((task = __pop_task(queue)) == NULL) // 尝试从队列中取出任务，直到返回任务不为 NULL
    {
        pthread_mutex_lock(&queue->mutex);
        if (atomic_load(&queue->block) == 0) // 如果是阻塞的
            return NULL;
        pthread_cond_wait(&queue->cond, &queue->mutex); // 等待 __add_task 将其唤醒
        pthread_mutex_unlock(&queue->mutex);
    }
    return task;
}

// 销毁队列
static void
__taskqueue_destroy(task_queue_t *queue)
{
    task_t *task;
    while ((task = __pop_task(queue)))
    {
        free(task);
        task = NULL;
    }
    pthread_spin_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond);
    pthread_mutex_destroy(&queue->mutex);

    free(queue);
    queue = NULL;
}

static void *
__threadpool_worker(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;
    task_t *task;
    void *ctx;

    while (atomic_load(&pool->quit) == 0)
    {
        task = (task_t *)__get_task(pool->task_queue);
        if (!task)
            break;
        handler_ptr func = task->func;
        ctx = task->arg;
        free(task);
        task = NULL;
        func(ctx);
    }
    return NULL;
}

static void
__threads_terminate(thread_pool_t *pool)
{
    atomic_store(&pool->quit, 1); // 设置退出标志
    __nonblock(pool->task_queue); // 设置任务队列为非阻塞

    for (int i = 0; i < pool->thread_count; ++i)
        pthread_join(pool->threads[i], NULL);
}

// 创建线程
static int __threads_create(thread_pool_t *pool, size_t threads_count)
{
    pthread_attr_t attr;
    int ret;

    ret = pthread_attr_init(&attr);
    if (ret == 0)
    {
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * threads_count);
        if (pool->threads)
        {
            int i = 0;
            for (; i < threads_count; ++i)
            {
                if (pthread_create(&pool->threads[i], &attr, __threadpool_worker, pool) != 0)
                    break;
            }
            pool->thread_count = i;
            pthread_attr_destroy(&attr);
            if (i == threads_count)
                return 0;
            __threads_terminate(pool);
            free(pool->threads);
        }
        ret = -1;
    }
    return -1;
}

// 线程池的创建
thread_pool_t *thread_pool_create(int threads_count)
{
    thread_pool_t *pool;
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (pool)
    {
        task_queue_t *queue = __taskqueue_create();
        if (queue)
        {
            pool->task_queue = queue;
            atomic_init(&pool->quit, 0);
            if (__threads_create(pool, threads_count) == 0)
                return pool;
            // __taskqueue_destroy(queue); // 2
            __taskqueue_destroy(pool->task_queue);
        }
        free(pool);
    }
    return NULL;
}

// 向线程池中添加一个任务
int thread_pool_post(thread_pool_t *pool, handler_ptr func, void *arg)
{
    if (atomic_load(&pool->quit) == 1)
        return -1;
    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (!task)
        return -1;
    task->func = func;
    task->arg = arg;
    __add_task(pool->task_queue, task);
    return 0;
}

// 暂停线程池
void thread_pool_terminate(thread_pool_t *pool)
{
    atomic_store(&pool->quit, 1);
    __nonblock(pool->task_queue);
}

void thread_pool_waitdone(thread_pool_t *pool)
{
    for (int i = 0; i < pool->thread_count; ++i)
        pthread_join(pool->threads[i], NULL);

    __taskqueue_destroy(pool->task_queue);
    free(pool->threads);
    free(pool);
}
