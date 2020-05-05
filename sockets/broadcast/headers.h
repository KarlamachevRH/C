#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#ifndef __HEADERS_H__
#define __HEADERS_H__

#define CLIENT_ADDR_LEN     16
#define DATE_STR_LEN        48

#define DEBUG

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