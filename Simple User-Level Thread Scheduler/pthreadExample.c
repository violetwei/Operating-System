// How to compile: gcc pthreadExample.c -l pthead && ./a.out

// A thread is a single sequence stream within in a process

// pthread_create() takes 4 arguments:
// first argument is a pointer to thread_id which is set by this function
// second argument specifies attributes. If the value is NULL, then default attributes shall be used
// third argument is name of function to be executed for the thread to be created
// fourth argument is used to pass arguments to the function

// pthread_join() - wait for thread termination, is a function for threads is the equivalent of wait() for processes

// Pthreads API - Mutexes
// Routines that deal with synchronization, called a "mutex", which is an abbreviation for "mutual exclusion"
// Mutex functions provide for creating, destroying, locking and unlocking mutexes

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

void *thread_1(void *arg) {
    pthread_mutex_t *m = arg;

    while (true) {
        pthread_mutex_lock(m);

        printf("1 Hello frrom\n");
        usleep(1000);

        printf("1 thread1\n");
        pthread_mutex_unlock(m);
        usleep(1000 * 1000);
    }
}

void *thread2(void *arg) {
    pthread_mutex_t *m = arg;

    while (true) {
        pthread_mutex_lock(m);

        printf("2 Hello frrom\n");
        usleep(1000);

        printf("2 thread2\n");
        pthread_mutex_unlock(m);
        usleep(1000 * 1000);
    }
}

int main() {
    pthread_t thread_handle;
    pthread_t thread2_handle;

    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

    /* Create independent threads each of which will execute function */
    pthread_create(&thread_handle, NULL, thread_1, &m);
    pthread_create(&thread2_handle, NULL, thread2, &m);

    pthread_join(thread_handle, NULL);
    pthread_join(thread2_handle, NULL);
}