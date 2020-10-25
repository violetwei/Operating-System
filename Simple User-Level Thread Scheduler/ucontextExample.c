// How to compile: gcc ucontextExample.c && ./a.out

#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
#include <stdbool.h>

/* static ucontext_t t1, t2, m;

void f1() {
    while (true) {
        usleep(1000 * 1000);
        printf("Hello from thread 1\n");
        swapcontext(&t1, &t2);
    }
}

void f2() {
    while (true) {
        usleep(1000 * 1000);
        printf("Hello from thread 2\n");
        swapcontext(&t2, &t1);
    }
}*/

void f1(void *main, void *self) {
    int numIter = 0;
    while (true) {
        usleep(1000 * 1000);
        printf("Hello from thread 1 called %d\n", numIter);
        numIter++;
        swapcontext(self, main);
    }
}

void f2(void *main, void *self) {
    while (true) {
        usleep(1000 * 1000);
        printf("Hello from thread 2\n");
        swapcontext(self, main);
    }
}

int main() {
    ucontext_t t1, t2, m;
    char f1stack[16 * 1024];
    char f2stack[16 * 1024];

    getcontext(&t1);
    t1.uc_stack.ss_sp = f1stack;
    t1.uc_stack.ss_size = sizeof(f1stack);
    t1.uc_link = &m;
    //makecontext(&t1, f1, 0);
    makecontext(&t1, (void(*)())f1, 2, &m, &t1);

    getcontext(&t2);
    t2.uc_stack.ss_sp = f2stack;
    t2.uc_stack.ss_size = sizeof(f2stack);
    t2.uc_link = &m;
    //makecontext(&t2, f2, 0);
    makecontext(&t2, (void(*)())f2, 2, &m, &t2);

    bool t1First = true;
    while (true) {
        if (t1First) {
            swapcontext(&m, &t1);
        } else {
            swapcontext(&m, &t2);
        }
        t1First = !t1First;
    }

    printf("exit from main\n");
    return 0;
}