#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // malloc()
#include <unistd.h> // sleep()
#include "thread_pool.h"

int done = 0;
pthread_mutex_t lock;

void do_task(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;
    pthread_mutex_lock(&lock);
    if (done >= 20)
    {
        thread_pool_terminate(pool);
    }
    else
    {
        done++;
        printf("doing %d task\n", done);
    }
    pthread_mutex_unlock(&lock);
}

int main()
{
    int threads = 10;
    pthread_mutex_init(&lock, NULL);

    thread_pool_t *pool = thread_pool_create(threads);
    if (pool == NULL)
    {
        perror("thread pool create error!\n");
        exit(-1);
    }

    while (thread_pool_post(pool, &do_task, pool) == 0)
    {}

    thread_pool_waitdone(pool);
    pthread_mutex_destroy(&lock);

    return 0;
}