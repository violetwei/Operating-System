#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "sut.h"
#include "./queue/queue.h"

threaddesc threadarr[MAX_THREADS];
int numthreads, curthread;
ucontext_t parentContext;
threaddesc *tdescptr;

// compute executor (cexec), responsible for creating tasks and launching them
pthread_t cexec;
// I/O executor (iexec)
pthread_t iexec;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct queue task_ready_q;
struct queue wait_q;

int taskCreated;
int taskCompleted;

// called by the user of the library before calling any other function
// used to perform initialization - create kernel level threads
void sut_init() {
    // initialize two kernel-level threads known as executors
    task_ready_q = queue_create();
    wait_q = queue_create();
    queue_init(&task_ready_q);
    queue_init(&wait_q);
    taskCreated = 0;
    taskCompleted = 0;
    
    pthread_create(&cexec, NULL, kernel_thread_1, NULL);
    pthread_create(&iexec, NULL, kernel_thread_2, NULL);
}

void *kernel_thread_1(void *arg) {
    // main kernel thread that work as c-exec
    printf("Starting c-exec \n");
    // wait unitl there is a thread created
    while (true) {
        pthread_mutex_lock(&mutex);
        if (numthreads != 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    while (true) {
        while (true) {
            struct queue_entry *popped;
            // acquie the lock and get the next task
            pthread_mutex_lock(&mutex);
            // if there is no task, break and check if there are live threads
            if (!queue_peek_front(&task_ready_q)) {
                pthread_mutex_unlock(&mutex);
                break;
            }
            popped = queue_pop_head(&task_ready_q);
            pthread_mutex_unlock(&mutex);
            //threaddesc *tdescptr;
            tdescptr = (threaddesc *)popped->data;
            swapcontext(&(parentContext), &(tdescptr->threadcontext));
            usleep(100);
            free(popped);
        }
        // acquire the lock and check if there are live threads. 
        pthread_mutex_lock(&mutex);
        if (numthreads == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("Exiting c-exec\n");
}

void *kernel_thread_2(void *arg) {
    
    pthread_mutex_lock(&mutex);
    printf("2 Hello from\n");
    usleep(1000);
    printf("2 thread2\n");
    pthread_mutex_lock(&mutex);
       
}

// called by the user of library in order to add a new task which should be scheduled to run on C-Exec thread
// parameter fn is a function the user would like to run in the task
bool sut_create(sut_task_f fn) {
    //threaddesc *tdescptr;

	if (numthreads >= MAX_THREADS) {
		printf("FATAL: Maximum thread limit reached... creation failed! \n");
		return -1;
	}
    curthread = numthreads;

	tdescptr = &(threadarr[numthreads]);
	getcontext(&(tdescptr->threadcontext));
	tdescptr->threadid = numthreads;
    printf("created thread with id %d\n", tdescptr->threadid);
	tdescptr->threadstack = (char *)malloc(THREAD_STACK_SIZE);
	tdescptr->threadcontext.uc_stack.ss_sp = tdescptr->threadstack;
	tdescptr->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	tdescptr->threadcontext.uc_link = &(parentContext);
	tdescptr->threadcontext.uc_stack.ss_flags = 0;
	tdescptr->threadfunc = fn;

	makecontext(&(tdescptr->threadcontext), *fn, 0);
	numthreads++;

    // a newly created task is added to the end of task ready queue
    struct queue_entry *node;

    // lock the queue, push, then unlock
    pthread_mutex_lock(&mutex);
    node = queue_new_node(tdescptr);
    queue_insert_tail(&task_ready_q, node);
    pthread_mutex_unlock(&mutex);

    swapcontext(&(parentContext), &(tdescptr->threadcontext));

	return true;
}

// called within a user task -> a task calls for pasusing its execution
// when called, the state/context of currently running task should be saved and rescheduled to be resumed later
// the C-Exec thread should be instructed to schedule the next task in its queue of ready task
void sut_yield() {
    taskCreated++;
    printf("Task created: %d\n", taskCreated);
   
    struct queue_entry *node = queue_new_node(tdescptr);

    //lock the queue, then push/pop, then unlock
    pthread_mutex_lock(&mutex);

    // a task that is yielding is put back in the end of task ready queue
    queue_insert_tail(&task_ready_q, node);
    
    if (queue_peek_front(&task_ready_q)) {
        return;
    }

    struct queue_entry *nextNode = queue_peek_front(&task_ready_q);
    pthread_mutex_unlock(&mutex);

    //struct __threaddesc *tdescptr = (threaddesc *)malloc(sizeof(struct __threaddesc));

    tdescptr = (threaddesc *)nextNode->data;
        
    swapcontext(&(parentContext), &(tdescptr->threadcontext));
        
    taskCompleted++;
    printf("Task completed: %d\n", taskCompleted);

}

// called within a user task
// when called, the state/context of the curently running task should be destroyed - should not resume this task again later
// the C-Exec thread should be instructed to schedule the next task in its queueof ready tasks
void sut_exit() {
//&& taskCreated != taskCompleted
}

// called within a user task
// when called, the I-Exec thread should be instructed to open a TCP socket connection to the address specified by dest on port
// the socket should be associated to the current task
// after signaling the I-Exec thread, the currently running task (the one called sut_open()) should have its state/conext saved
// the task should not be resumed until the I-Exec thread has completed the operation
// meanwhile, the C-Exec thread should be instructed to schedule the next task in its queue of ready tasks
void sut_open(char *dest, int port) {

}

// called within a user task
// when called, the I-Exec thread should be instructed to write size bytes from buf to the socket assoiciated with the current task
// non-blocking wrtie
// after I-Exec thread is signaled to perform the write, the callling task should continue on the C-Exec thread and not be interrrupted
// the write should be performed by the I-Exec thread concurrently with the still-running calling task on C-Exec thread
void sut_write(char *buf, int size) {

}

// called within a user task
// when called, the I-Exec thread should close the socket associated with the current task
// non-blocking close
// after the I-Exec thread is signaled to close the socket, the calling task should continue on the C-Exec thread and not be interrupted
void sut_close() {

}

// called within a user task
// when called, the I-Exec thread should be instructed to read from task's associated socket until there is no more data to read
// after signaling the I-Exec thread to perform the read, the currently running task (the one callled sut_read()) should have its state/context saved
// the task should not be resumed until the I-Exec thread has completed the read operation.
// meanwhile, the C-Exec thread should be instructed to schedule the next task in its queue of ready tasks
char *sut_read() {

}

void sut_shutdown() {
    pthread_join(cexec, NULL);
    pthread_join(iexec, NULL);
}
