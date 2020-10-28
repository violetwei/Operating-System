#ifndef __SUT_H__
#define __SUT_H__
#include <stdbool.h>
#include <ucontext.h>

#define MAX_THREADS                        32
#define THREAD_STACK_SIZE                  1024*64


typedef struct __threaddesc
{
	int threadid;
	char *threadstack;
	void *threadfunc;
	ucontext_t threadcontext;
} threaddesc;



extern threaddesc threadarr[MAX_THREADS];
extern int numthreads, curthread;
extern ucontext_t parent;


typedef void (*sut_task_f)();

void *kernel_thread_1(void *arg);
void *kernel_thread_2(void *arg);

void sut_init();
bool sut_create(sut_task_f fn);
void sut_yield();
void sut_exit();
void sut_open(char *dest, int port);
void sut_write(char *buf, int size);
void sut_close();
char *sut_read();
void sut_shutdown();


#endif
