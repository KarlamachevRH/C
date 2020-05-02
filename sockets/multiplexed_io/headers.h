#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>

#ifndef __HEADERS_H__
#define __HEADERS_H__

#define MAX_BUF_SIZE        128
#define QUEUE_SIZE          5
#define CLIENT_ADDR_LEN     16

#define DEBUG
//#define SELECT
//#define POLL
//#define EPOLL

#ifdef DEBUG
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif //DEBUG

#define handle_error(msg)           \
        do                          \
        {                           \
            perror(msg);            \
            exit(EXIT_FAILURE);     \
        } while(0)

#endif // HEADERS_H