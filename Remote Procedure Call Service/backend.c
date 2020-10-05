#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "a1_lib.h"
#include "backend.h"
#include "rpc.h"

#define BUFSIZE   1024
#define MAX_CLIENT  5

int main(int argc, char* argv[]) {
  int sockfd, clientfd;
  char msg[BUFSIZE];
  const char *greeting = "hello, world\n";
  char result[BUFSIZE];
  int running = 1;
  int pid;
  int rval;
  bool finished = false;

  //if (create_server("0.0.0.0", 10000, &sockfd) < 0) {
  if (create_server(argv[1], atoi(argv[2]), &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  } else {
    printf("Server listening on %s:%s\n\n", argv[1], argv[2]);
  }

  // set socket to non-blocking
  /*if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
    return -1;
  }*/

  while (!finished) {
    if (accept_connection(sockfd, &clientfd) < 0) {
      fprintf(stderr, "oh no\n");
      exit(0);
    }
    printf("New Connection Accepted!\n\n");
    if ((pid = fork()) == 0) { // child process
      printf("Process id %d running\n\n", getpid());
      sleep(2);
      while (strcmp(msg, "quit\n")) {
        memset(msg, 0, sizeof(msg));
        ssize_t byte_count = recv_message(clientfd, msg, BUFSIZE);

        // restore input message to a struct
        struct message_t *input_message = (struct message_t*) msg;
        char *cmd = input_message->command;

        if (strcmp(cmd, "add") == 0) {
          int additionAns = addInts(input_message->integer1, input_message->integer2);
          //itoa(additionAns, result, 10);
          snprintf(result, sizeof(result), "%d\n", additionAns);
        } else if (strcmp(cmd, "multiply") == 0) {
          int multiplyAns = multiplyInts(input_message->integer1, input_message->integer2);
          // itoa(multiplyAns, result, 10);
          snprintf(result, sizeof(result), "%d\n", multiplyAns);
        } else if (strcmp(cmd, "divide") == 0) {
          float divisionAns = divideFloats(input_message->float1, input_message->float2);
          if (input_message->float2 == 0) {
            strcpy(result, "Error: Division by zero\n");
          } else {
            //ftoa(divisionAns, result, 6);
            snprintf(result, sizeof(result), "%.6f\n", divisionAns);
          }
        } else if (strcmp(cmd, "factorial") == 0) {
          int factAns = factorial(input_message->factNum);
          // printf("Factorial number is %d", input_message->factNum);
          //itoa(factAns, result, 10);
          snprintf(result, sizeof(result), "%d\n", factAns);
        } else if (strcmp(cmd, "quit\n") == 0) {
            strcpy(result, "Bye!\n");
            //close(sockfd);
            finished = true;
            shutdown(sockfd, SHUT_RDWR);
            return 0;
        } else if (strcmp(cmd, "sleep") == 0) {
          sleepXSeconds(input_message->seconds);
          strcpy(result, "\n");
        } else if (strcmp(cmd, "exit\n") == 0) { // terminates the frontend only
          
          strcpy(result, "Goodbye frontend!\n");
        } else {
            snprintf(result, sizeof(result), "Error: Command \"%s\" not found\n", cmd);
        }
        if (byte_count <= 0) {
          break;
        }
        printf("Client command: %s\n\n", msg);
        send_message(clientfd, result, strlen(result));
      }
    } else { // parent process
        close(clientfd);
    }
  }

  return 0;
}


// add two integers
int addInts(int a, int b) {
    int ans = a + b;
    printf("Addition result: %d\n", ans);
    return ans;
}

// multiply two integers
int multiplyInts(int a, int b) {
    int ans = a * b;
    printf("Multiplication result: %d\n", ans);
    return ans;
}

// divide float numbers & report divide by zero error
// output: a floating point number with exactly 6 digits after the decimal point or Error: Division by zero
float divideFloats(float a, float b) {
    if (b == 0) {
        printf("Error: Division by zero\n");
        return 0;
    } else {
        float ans = a / b;
        printf("Division result: %.6f\n", ans);
        return ans;
    }
}

// make the calculator sleep for x seconds - this is blocking
int sleepXSeconds(int x) {
    printf("Sleep %d seconds\n", x);
    sleep(x);
    return x;
}

// x is guaranteed to be in the range [0, 20]
// return factorial of x
uint64_t factorial(int x) {
  int fact = 1;
    for (int i = 1; i <= x; i++) {
        fact *= i;
    }
    printf("Factorial result of %d: %d\n", x, fact);
    return fact;
}
