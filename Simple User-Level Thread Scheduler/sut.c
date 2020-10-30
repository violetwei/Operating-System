#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "sut.h"
#include "queue.h"


static int numthreads;
ucontext_t parentContext;
static threaddesc *cur_tdescptr;

// compute executor (cexec), responsible for creating tasks and launching them
static pthread_t cexec;
// I/O executor (iexec)
static pthread_t iexec;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t io_lock = PTHREAD_MUTEX_INITIALIZER;

struct queue task_ready_q;
struct queue wait_q;
struct queue io_to_q;
struct queue io_from_q;

static bool cexecRunning = true;
static bool iexecRunning = true;

int taskCreated;
int taskCompleted;

int total_byte_read;

ssize_t byte_read;
ssize_t byte_sent;

#define BUFSIZE  1024

typedef struct connection {
    char *ip;
    int port;
    int sockfd;
    // read context
    char read_buffer[BUFSIZE];
    char write_buffer[BUFSIZE];
    int write_size;
    bool can_read;
    bool can_write;
    bool open;
} connection;

struct connection *tcp_connect;


void shutdown_cexec_routine();
void shutdown_iexec_routine();

// called by the user of the library before calling any other function
// used to perform initialization - create kernel level threads
void sut_init() {
    numthreads = 0;
    total_byte_read = 0;

    // initialize two kernel-level threads known as executors
    task_ready_q = queue_create();
    wait_q = queue_create();
    io_to_q = queue_create();
    io_from_q = queue_create();

    queue_init(&task_ready_q);
    queue_init(&wait_q);
    queue_init(&io_to_q);
    queue_init(&io_from_q);

    taskCreated = 0;
    taskCompleted = 0;

    tcp_connect = (connection *)malloc(sizeof(connection));
    // initialize the open connection state to false
    tcp_connect->open = false;
    tcp_connect->can_read = false;
    tcp_connect->can_write = false;
    
    // create pthread
    pthread_create(&cexec, NULL, cexec_thread_run, 0);
    pthread_create(&iexec, NULL, iexec_thread_run, 0);
}


void *cexec_thread_run(void *args) {
    printf("Starting C-Exec\n");
    while(cexecRunning) {
        while (queue_peek_front(&task_ready_q)) {
            // acquire lock, then pop queue operation, then unlock
            pthread_mutex_lock(&mutex);
            cur_tdescptr = (threaddesc *)(queue_pop_head(&task_ready_q)->data);
            pthread_mutex_unlock(&mutex);

            cur_tdescptr->threadcontext.uc_link = &parentContext;
            swapcontext(&(parentContext), &(cur_tdescptr->threadcontext));

            // issue a usleep for 100 microseconds
            // after the sleep, cexec will check the ready queue again
            usleep(100);
        }
    }
    printf("Exiting C-Exec\n");

    // signal to iexec 
    shutdown_iexec_routine();
}


void *iexec_thread_run(void *arg) {
    printf("Starting I-Exec\n");
    while (iexecRunning) {
        // wait for tcp socket connection signaled by sut_open
        while (iexecRunning) {
            if (tcp_connect->open) {
               
                struct sockaddr_in server_address = { 0 };

                // create TCP socket & connect to server
                tcp_connect->sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (tcp_connect->sockfd < 0) {
                    perror("Error opening socket");
                }
                printf("Socket opened successfully!\n");

                // connect to server
                server_address.sin_family = AF_INET;
                inet_pton(AF_INET, tcp_connect->ip, &(server_address.sin_addr.s_addr));
                server_address.sin_port = htons(tcp_connect->port);
                if (connect(tcp_connect->sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
                    perror("Failed to connect to server\n");
                } else {
                    printf("Connected to server!\n");
                }

                pthread_mutex_lock(&io_lock);
                // resume the task
                struct queue_entry *node= queue_new_node((threaddesc *)(queue_pop_head(&wait_q)->data));
                pthread_mutex_unlock(&io_lock);

                pthread_mutex_lock(&mutex);
                //transfer the task back to ready queue, rescheduled
                queue_insert_tail(&task_ready_q, node);
                pthread_mutex_unlock(&mutex);

                break;
            }
        }
        // wait to read from the task's associated socket
        while (iexecRunning && tcp_connect->open) {
            // iexec is signaled to read
            if (tcp_connect->can_read) {
                //printf("Reached read in iexec!\n");
                /*if (byte_read < BUFSIZE) {
                    int count = recv(tcp_connect->sockfd, &(tcp_connect->read_buffer)[byte_read], BUFSIZE-byte_read, 0);
                    if (count <= 0) {
                        fprintf(stderr, "%s\n", strerror(errno));
                    }
                    byte_read += count;
                    printf("Byte read %d\n", byte_read);
                }*/
                
                byte_read = recv(tcp_connect->sockfd, tcp_connect->read_buffer, BUFSIZE, 0);
                total_byte_read += byte_read;
                //printf("Byte read %d\n", byte_read);
                //printf("Total Byte read %d\n", total_byte_read);
                if (byte_read <= 0) {
                    //fprintf(stderr, "recv failed %s\n", strerror(errno));
                    perror("recv failed: ");
                }

                // set the read flag back to false
                tcp_connect->can_read = false;

                //printf("Read buffer: %s\n", tcp_connect->read_buffer);
                    
                // when the response arrives, the task is moved from wait queue to task ready queue
                pthread_mutex_lock(&io_lock);
                // resume the task
                struct queue_entry *node= queue_new_node((threaddesc *)(queue_pop_head(&wait_q)->data)); 
                pthread_mutex_unlock(&io_lock);

                pthread_mutex_lock(&mutex);
                //transfer the task back to ready queue, rescheduled
                queue_insert_tail(&task_ready_q, node);
                pthread_mutex_unlock(&mutex);
            }

            // iexec is signaled to write
            if (tcp_connect->can_write) {
                send(tcp_connect->sockfd, tcp_connect->write_buffer, tcp_connect->write_size, 0);
                printf("IEXEC has sent %s to the socket!!\n", tcp_connect->write_buffer);
                // set the write flag back to false
                tcp_connect->can_write = false;
            }
        }

        // wait to close the socket associated with current task
        while (iexecRunning) {
            if (tcp_connect->open == false) {
                pthread_mutex_lock(&io_lock);
                // close the socket
                int n = close(tcp_connect->sockfd);
                pthread_mutex_unlock(&io_lock);
                printf("Socket Closed!!! %d\n", n);
                break;
            }
        }
    }
    
    printf("Exiting I-Exec\n");   
}


void shutdown_iexec_routine() {
    iexecRunning = false;
}

void shutdown_cexec_routine() {
    cexecRunning = false;
}



// called by the user of library in order to add a new task which should be scheduled to run on C-Exec thread
// parameter fn is a function the user would like to run in the task
bool sut_create(sut_task_f fn) {
    threaddesc *tdescptr = (threaddesc *)malloc(sizeof(threaddesc));

	if (numthreads >= MAX_THREADS) {
		printf("FATAL: Maximum thread limit reached... creation failed! \n");
		return false;
	}

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

    // cexec schedules next available task
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
    // instruct cexec to schedule next task in ready queue
    swapcontext(&cur_context, &next_context);
}


// called within a user task
// when called, the I-Exec thread should be instructed to open a TCP socket connection to the address specified by dest on port
// the socket should be associated to the current task
// after signaling the I-Exec thread, the currently running task (the one called sut_open()) should have its state/conext saved
// the task should not be resumed until the I-Exec thread has completed the operation
// meanwhile, the C-Exec thread should be instructed to schedule the next task in its queue of ready tasks
void sut_open(char *dest, int port) {
    // signal iexec to open a TCP socket connection
    tcp_connect->ip = dest;
    tcp_connect->port = port;
    tcp_connect->open = true;
 
    printf("signal iexec to open connection!\n");
    // the currently running task that called sut_open should not be resumed until iexec completes the operation
    struct queue_entry *node = queue_new_node(cur_tdescptr);
    // the task stays in wait queue until exec completes the operation
    queue_insert_tail(&wait_q, node);

    // instruct cexec thread to schedule the next task in ready queue
    swapcontext(&(cur_tdescptr->threadcontext), (cur_tdescptr->threadcontext).uc_link);

    // printf("sut_open finished!\n");
}


// called within a user task
// when called, the I-Exec thread should be instructed to write size bytes from buf to the socket assoiciated with the current task
// non-blocking wrtie
// after I-Exec thread is signaled to perform the write, the callling task should continue on the C-Exec thread and not be interrrupted
// the write should be performed by the I-Exec thread concurrently with the still-running calling task on C-Exec thread
void sut_write(char *buf, int size) {
    if (tcp_connect->open != true) {
        printf("Error! The task has not successfully called sut_open()!\n");
        exit(1);
    }
    // save the buf and size in the tcp_connect struct
    strcpy(tcp_connect->write_buffer, buf);
    tcp_connect->write_size = size;

    // signal iexec to write to the socket assiciated with current task
    tcp_connect->can_write = true;
}


// called within a user task
// when called, the I-Exec thread should close the socket associated with the current task
// non-blocking close
// after the I-Exec thread is signaled to close the socket, the calling task should continue on the C-Exec thread and not be interrupted
void sut_close() {
    // terminate the connection we've established with the remote process
    if (tcp_connect->open != true) {
        printf("Error! The task has not successfully called sut_open()!\n");
        exit(1);
    }
    // signal iexec to close the socket associated with current task in iexec
    tcp_connect->open = false;
}


// called within a user task
// when called, the I-Exec thread should be instructed to read from task's associated socket until there is no more data to read
// after signaling the I-Exec thread to perform the read, the currently running task (the one callled sut_read()) should have its state/context saved
// the task should not be resumed until the I-Exec thread has completed the read operation.
// meanwhile, the C-Exec thread should be instructed to schedule the next task in its queue of ready tasks
char *sut_read() {
    if (tcp_connect->open != true) {
        printf("Error! The task has not successfully called sut_open()!\n");
        exit(1);
    }
    
    // signal the iexec thread to read from task's associated socket
    tcp_connect->can_read = true;
    
    printf("Signal iexec to read!\n");

    struct queue_entry *node = queue_new_node(cur_tdescptr);
    // put the task in the back of wait queue, it stays there until iexec completes read
    queue_insert_tail(&wait_q, node);

    // instruct cexec thread to schedule the next task in ready queue
    swapcontext(&(cur_tdescptr->threadcontext), (cur_tdescptr->threadcontext).uc_link);

    // can_read is set to false after iexec finishes read operation
    while (tcp_connect->can_read) {
        // wait for the recv() to fill the read_buffer;
    }

    return tcp_connect->read_buffer;
}


// called by the user when they are done adding tasks and would like to wait for currently running tasks to finish
void sut_shutdown() {
    sut_create(shutdown_cexec_routine);

    pthread_join(iexec, NULL);
    pthread_join(cexec, NULL);
}
