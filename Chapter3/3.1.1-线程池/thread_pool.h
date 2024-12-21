#ifndef _THREAD_POLL_H
#define _THREAD_POLL_H

typedef struct thread_pool_s thread_pool_t;
typedef void (*handler_ptr)(void * /* context */);

// c++ 也能使用这个线程池
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// 线程池的创建
thread_pool_t *thread_pool_create(int threads_count);
// 向线程池中添加一个任务
int thread_pool_post(thread_pool_t *pool, handler_ptr func, void *arg);
// 暂停线程池
void thread_pool_terminate(thread_pool_t *pool);
// 等待所有线程完成任务后，退出
void thread_pool_waitdone(thread_pool_t *pool);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_THREAD_POLL_H