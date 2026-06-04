#ifndef THREAPOOL_H
#define THREAPOOL_H

#include <pthread.h>

typedef struct {

    void (*function)(void *);
    void *arg;

} threadpool_task_t;

typedef struct  {
    
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    threadpool_task_t *queue;
    
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;

} ThreadPoolManager;

ThreadPoolManager *threadpool_create(int thread_count, int queue_size);
int threadpool_add(ThreadPoolManager *pool, void (*function)(void *), void *arg);
int threadpool_destroy(ThreadPoolManager *pool);

#endif 