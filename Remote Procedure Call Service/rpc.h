#ifndef MESSAGE_H_
#define MESSAGE_H_

#define CMD_LENGTH   256
#define ARGS_LENGTH  256

typedef struct message_t {
    char command[CMD_LENGTH];
    int integer1;
    int integer2;
    float float1;
    float float2;
    int factNum;
    int seconds;
    //char args[ARGS_LENGTH];
} message_t;

#endif // MESSAGE_H_