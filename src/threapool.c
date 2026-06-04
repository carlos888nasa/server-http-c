#include <stdio.h>
#include <stdlib.h>

#include "../include/threapool.h"

static void *threadpool_thread(void *threadpool){

    ThreadPoolManager *pool = (ThreadPoolManager *) threadpool;

    while(1){

        pthread_mutex_lock(&(pool->lock));

        while((pool->count == 0) && (!pool->shutdown)){

            pthread_cond_wait(&(pool->notify), &(pool->lock));

        }

        if((pool->shutdown)){

            pthread_mutex_unlock(&(pool->lock));
            break;

        }

        threadpool_task_t task;
        task.function = pool->queue[pool->head].function;
        task.arg = pool->queue[pool->head].arg;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->lock));

        (*(task.function))(task.arg);

    }

    return NULL;

}

ThreadPoolManager *threadpool_create(int thread_count, int queue_size){

    ThreadPoolManager *pool = malloc(sizeof(ThreadPoolManager));

    if(pool == NULL){

        return NULL;

    }

    pool->threads = malloc(sizeof(pthread_t) * thread_count);

    if(pool->threads == NULL){

        free(pool);
        return NULL;

    }

    pool->queue = malloc(sizeof(threadpool_task_t) * queue_size);

    if(pool->queue == NULL){

        free(pool->threads);
        free(pool);
        return NULL;

    }


    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->shutdown = 0;

    if(pthread_mutex_init(&(pool->lock), NULL) != 0 || pthread_cond_init(&(pool->notify), NULL) != 0){

        free(pool->threads);
        free(pool->queue);
        free(pool);
        return NULL;

    }

    for(int i = 0; i < thread_count; i++){

        if(pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *) pool) != 0){

            threadpool_destroy(pool);
            return NULL;

        }


    }

    return pool;

}

int threadpool_add(ThreadPoolManager *pool, void(*function)(void * ), void *args){

    if(pool == NULL || function == NULL){

        return -1;

    }

    if(pthread_mutex_lock(&(pool->lock)) != 0){

        return -1;

    }

    int error = 0;

    if(pool->count == pool->queue_size){

        error = -1;

    } else if(pool->shutdown){

        error = -1;

    } else {

        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].arg = args;
        pool->tail = (pool->tail + 1) % pool->queue_size;
        pool->count += 1;

        pthread_cond_signal(&(pool->notify));


    }

    pthread_mutex_unlock(&(pool->lock));

    return error;
}

int threadpool_destroy(ThreadPoolManager *pool){

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for(int i = 0; i < pool->thread_count; i++){

        pthread_join(pool->threads[i], NULL);

    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));

    free(pool->queue);
    free(pool->threads);
    free(pool);

    return 0;

}