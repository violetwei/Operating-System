#include <stdio.h>
#include <stdlib.h>

// send struct as messages

struct message {
    int x;
    float y;
    char s[1024];
};

// print integer
void printInteger(const char *buf) {
    int *x = (int*) buf;
    printf("%d\n", *x);
}

// print message
void printMessage(const char *buf) {
    struct message *msg = (struct message*) buf;
    printf("%d %f %s\n", msg->x, msg->y, msg->s);
}

int main(void) {
    int x = 10;
    struct message m = {10, 1.5, "hello, world"};
    printInteger((char*)&x)
    printMessage((char*)&m);
    return 0;
}