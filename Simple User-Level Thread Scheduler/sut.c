#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "sut.h"
#include "queue/queue.h"

//threaddesc threadarr[MAX_THREADS];
static int numthreads;
ucontext_t parentContext;
static threaddesc *cur_tdescptr;

// compute executor (cexec), responsible for creating tasks and launching them
static pthread_t cexec;
// I/O executor (iexec)
static pthread_t iexec;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct queue task_ready_q;
struct queue wait_q;

static bool isRunning = true;

int taskCreated;
int taskCompleted;

typedef struct connection {
    char *ip;
    int port;
    //boolean open = false;
} connection;


void shutdown_routine_helper();

// called by the user of the library before calling any other function
// used to perform initialization - create kernel level threads
void sut_init() {
    numthreads = 0;

    // initialize two kernel-level threads known as executors
    task_ready_q = queue_create();
    wait_q = queue_create();
    queue_init(&task_ready_q);
    queue_init(&wait_q);

    taskCreated = 0;
    taskCompleted = 0;
    
    // create pthread
    pthread_create(&cexec, NULL, cexec_thread_run, 0);
    pthread_create(&iexec, NULL, iexec_thread_run, 0);
}

void *cexec_thread_run(void *args) {
    while(isRunning) {
        while (queue_peek_front(&task_ready_q)) {
            // acquire lock, then pop queue operation, then unlock
            pthread_mutex_lock(&mutex);
            cur_tdescptr = (threaddesc *)(queue_pop_head(&task_ready_q)->data);
            pthread_mutex_unlock(&mutex);
            cur_tdescptr->threadcontext.uc_link = &parentContext;
            swapcontext(&(parentContext), &(cur_tdescptr->threadcontext));
        }
    }
}

void *iexec_thread_run(void *arg) {
    
    /*pthread_mutex_lock(&mutex);
    printf("2 Hello from\n");
    usleep(1000);
    printf("2 thread2\n");
    pthread_mutex_lock(&mutex);*/
       
}

void shutdown_routine_helper() {
    isRunning = false;
}

// called by the user of library in order to add a new task which should be scheduled to run on C-Exec thread
// parameter fn is a function the user would like to run in the task
bool sut_create(sut_task_f fn) {
    threaddesc *tdescptr = (threaddesc *)malloc(sizeof(threaddesc));

	if (numthreads >= MAX_THREADS) {
		printf("FATAL: Maximum thread limit reached... creation failed! \n");
		return false;
	}

	//tdescptr = &(threadarr[numthreads]);
	getcontext(&(tdescptr->threadcontext));
	tdescptr->threadid = numthreads;
    //printf("created thread with id %d\n", tdescptr->threadid);
	tdescptr->threadstack = (char *)malloc(THREAD_STACK_SIZE);
	tdescptr->threadcontext.uc_stack.ss_sp = tdescptr->threadstack;
	tdescptr->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	tdescptr->threadcontext.uc_link = &(parentContext);
	tdescptr->threadcontext.uc_stack.ss_flags = 0;
	tdescptr->threadfunc = fn;

	makecontext(&(tdescptr->threadcontext), fn, 0);
	numthreads++;

    // a newly created task is added to the end of task ready queue
    struct queue_entry *node= queue_new_node(tdescptr);
  
    queue_insert_tail(&task_ready_q, node);
    
	return true;
}

// called within a user task -> a task calls for pasusing its execution
// when called, the state/context of currently running task should be saved and rescheduled to be resumed later
// the C-Exec thread should be instructed to schedule the next task in its queue of ready task
void sut_yield() {
    taskCreated++;
    printf("Task created: %d\n", taskCreated);
   
    struct queue_entry *node = queue_new_node(cur_tdescptr);

    // a task that is yielding is put back in the end of task ready queue
    queue_insert_tail(&task_ready_q, node);

    swapcontext(&(cur_tdescptr->threadcontext), (cur_tdescptr->threadcontext).uc_link);
        
    //taskCompleted++;
    //printf("Task completed: %d\n", taskCompleted);

}

// called within a user task
// when called, the state/context of the curently running task should be destroyed - should not resume this task again later
// the C-Exec thread should be instructed to schedule the next task in its queueof ready tasks
void sut_exit() {
    numthreads--;
    ucontext_t cur_context = cur_tdescptr->threadcontext;
    ucontext_t next_context = *((cur_tdescptr->threadcontext).uc_link);
    free(cur_tdescptr->threadstack);
    free(cur_tdescptr);
    swapcontext(&cur_context, &next_context);
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
    sut_create(shutdown_routine_helper);
    pthread_join(iexec, NULL);
    pthread_join(cexec, NULL);
}
