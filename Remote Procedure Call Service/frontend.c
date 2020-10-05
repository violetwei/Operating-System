#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "a1_lib.h"
#include "frontend.h"
#include "rpc.h"

#define BUFSIZE   1024

//struct message_t parseToStruct(char user_input[]);

int main(int argc, char* argv[]) {
  int sockfd;
  char user_input[BUFSIZE] = { 0 };
  char server_msg[BUFSIZE] = { 0 };

  //if (connect_to_server("0.0.0.0", 10000, &sockfd) < 0) {
  if (connect_to_server(argv[1], atoi(argv[2]), &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }

  while (strcmp(user_input, "quit\n")) {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));

    printf(">> ");

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);

    // convert the input to a struct message
    struct message_t message_input = parseToStruct(user_input); // (char*)&user_input

    // send the input to server
    send_message(sockfd, (char*)&message_input, sizeof(message_input));

    // receive a msg from the server
    ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
    
    if (byte_count <= 0) {
      break;
    }
    
    printf("%s\n", server_msg);
  }

  return 0;
}

/*int main(int argc, char* argv[]) {
  int sockfd;
  char user_input[BUFSIZE] = { 0 };
  char server_msg[BUFSIZE] = { 0 };

  if (argc != 3) {
    printf("Please enter the required arguments correctly!");
    return -1;
  }

  char *host_ip = argv[1];
  int host_port = atoi(argv[2]);

  // connect_to_server(const char *host, uint16_t port, int *sockfd)
  // connect_to_server("0.0.0.0", 10000, &sockfd)
  // ./frontend <host_ip> <host_port>
  if (connect_to_server(host_ip, host_port, &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }

  if (strcmp(user_input, "quit\n") == 0) {
    printf("Bye!");
  }

  while (strcmp(user_input, "quit\n")) {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);

    // convert the input to a struct message
    struct message_t message_input = parseToStruct(user_input); // (char*)&user_input

    // send the input to server
    send_message(sockfd, (char*)&message_input, sizeof(message_input));

    // receive a msg from the server
    ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
    
    if (byte_count <= 0) {
      break;
    }
    printf("Server: %s\n", server_msg);
  }

  return 0;
}*/

struct message_t parseToStruct(char user_input[]) {
  char *command = strtok(user_input, " ");
  struct message_t message;
  strcpy(message.command, command);
  if (strcmp(command, "add") == 0) {
    message.integer1 = atoi(strtok(NULL, " "));
    message.integer2 = atoi(strtok(NULL, " "));
  } else if (strcmp(command, "multiply") == 0) {
    message.integer1 = atoi(strtok(NULL, " "));
    message.integer2 = atoi(strtok(NULL, " "));
  } else if (strcmp(command, "divide") == 0) {
    message.float1 = atof(strtok(NULL, " "));
    message.float2 = atof(strtok(NULL, " "));
  } else if (strcmp(command, "factorial") == 0) {
    message.factNum = atoi(strtok(NULL, " "));
  } else if (strcmp(command, "sleep") == 0) {
    message.seconds = atoi(strtok(NULL, " "));
  }
  return message;
}
