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
#include <arpa/inet.h>
#include <netdb.h> 
#include <ifaddrs.h>
#include <sys/errno.h>
#include "amessage.pb-c.h"

#define h_addr  h_addr_list[0]
#define MAX_SEND_SIZE 1024 			//max size of text buffer in data structure
#define BUFF_SIZE 32 			// buffer for text in message from udp server
#define PROCESS_NUM 2

char *msgToRecieve = "Waiting messages!";
char *msgToSend = "Messages in the queue!";
#endif