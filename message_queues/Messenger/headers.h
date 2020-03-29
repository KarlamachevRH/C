#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <signal.h>
#include <mqueue.h>

#ifndef __HEADERS_H__
#define __HEADERS_H__

#define DEBUG

#define MAX_BUF_SIZE 128

#ifdef DEBUG
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif //DEBUG

#define name "/mq"
#define NAME_LEN 32
#define MSG_LEN 256

struct client_msg
{
    char client_name[NAME_LEN];
    char msg[MSG_LEN];
};

void send_message(mqd_t mqid, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);	            //отправить данные
void read_message(mqd_t mqid, struct client_msg *msg_ptr, size_t msg_len, unsigned int *msg_prio);		//принять данные

#endif // HEADERS_H