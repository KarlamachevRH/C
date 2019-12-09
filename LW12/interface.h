#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#define _XOPEN_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/errno.h>

#define h_addr  h_addr_list[0]
#define MAX_SEND_SIZE 1024 			//max size of text buffer in data structure
#define BUFF_SIZE 256 				// buffer for text in message from udp server
#define PROCESS_NUM 2

//структура для помещения данных отправленных/полученных по TCP
struct TCPmsgbuf {	
	int t;
	int len;
	char *mtext;
}__attribute__((packed));

char *msgToRecieve = "Waiting messages!";
char *msgToSend = "Messages in the queue!";
#endif