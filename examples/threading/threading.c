#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    int result;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    thread_func_args->thread_complete_success = false;
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    result = pthread_mutex_lock(thread_func_args->mutex);
    if (result != 0)
    {
        ERROR_LOG("Mutex lock failed with result %d",result);
        return thread_func_args;
    }
    usleep(thread_func_args->wait_to_release_ms * 1000);
    result = pthread_mutex_unlock(thread_func_args->mutex);
    if (result != 0)
    {
        ERROR_LOG("Mutex unlock failed with result %d",result);
        return thread_func_args;
    }
    thread_func_args->thread_complete_success = true;
    return thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data* thread_data_ = malloc(sizeof(struct thread_data));
    thread_data_->mutex = mutex;
    thread_data_->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data_->wait_to_release_ms = wait_to_release_ms;

    int result = pthread_create(thread, NULL, threadfunc, (void*)thread_data_);
    if (result != 0)
    {
        ERROR_LOG("Cannot create thread with result %d",result);
        return false;
    }
    return true;
}

