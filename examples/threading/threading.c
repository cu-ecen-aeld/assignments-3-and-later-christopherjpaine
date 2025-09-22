#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
// #define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    thread_func_args->thread_complete_success = false;

    // wait 
    DEBUG_LOG("Waiting in Thread...");
    usleep(thread_func_args->wait_to_obtain_ms * 1000);
    
    // obtain
    DEBUG_LOG("Locking...");
    pthread_mutex_lock(thread_func_args->mutex);

    // wait
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // release 
    DEBUG_LOG("Unlocking...");
    pthread_mutex_unlock(thread_func_args->mutex);

    thread_func_args->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{

    /* Allocate thread data on the heap. */
    struct thread_data* data = malloc(sizeof(struct thread_data));
    if (!data) {
        return false;
    }
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;

    /* Create thread */
    DEBUG_LOG("Creating Thread");
    int ret;
    ret = pthread_create (thread, NULL, threadfunc, data);
    if (ret) {
        ERROR_LOG("pthread_create failed: %d", ret);
        return false;
    } else {
        return true;
    }
}

