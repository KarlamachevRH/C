#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h>

#ifndef __HEADERS_H__
#define __HEADERS_H__

#define MSG_QUEUE_SYSTEM_V
//#define MSG_QUEUE_POSIX

#ifdef MSG_QUEUE_SYSTEM_V
#include <sys/msg.h>
#endif // MSG_QUEUE_SYSTEM_V

#ifdef MSG_QUEUE_POSIX
#include <mqueue.h>
#endif // MSG_QUEUE_POSIX

#define DEBUG

#define MAX_BUF_SIZE 128

#ifdef DEBUG
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif //DEBUG

#ifdef MSG_QUEUE_POSIX
#define name "/mq"
enum msg_prio
{
    SERVER_MSG_PRIO,
    CLIENT_MSG_PRIO
};
void send_message(mqd_t mqid, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);	//отправить данные
void read_message(mqd_t mqid, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);		//принять данные
#endif // MSG_QUEUE_POSIX

#ifdef MSG_QUEUE_SYSTEM_V
key_t key;
#define path_name "server"
#define proj_id 'A'
enum msg_type
{
    SERVER_MSG_TYPE = 1,
    CLIENT_MSG_TYPE
};
struct msgbuf 
{
    long mtype;
    char mtext[MAX_BUF_SIZE];
};
void send_message(int qid, struct msgbuf *qbuf, long type, char *text);	//отправить данные
void read_message(int qid, struct msgbuf *qbuf, long type);				//принять данные
#endif // MSG_QUEUE_SYSTEM_V

#endif // HEADERS_H