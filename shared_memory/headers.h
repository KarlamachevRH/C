#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h>

#ifndef __HEADERS_H__
#define __HEADERS_H__

#define MAX_BUF_SIZE 128


#define DEBUG

#ifdef DEBUG
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif //DEBUG

#define SHARED_MEMORY_SYSTEM_V
//#define SHARED_MEMORY_POSIX

#ifdef SHARED_MEMORY_SYSTEM_V
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif // SHARED_MEMORY_SYSTEM_V

#ifdef SHARED_MEMORY_POSIX
#include <semaphore.h>
#include <sys/mman.h>
#endif // SHARED_MEMORY_POSIX


#ifdef SHARED_MEMORY_SYSTEM_V

int sem_id;
union semun 
{
    int val;                  /* значение для SETVAL */
    struct semid_ds *buf;     /* буферы для  IPC_STAT, IPC_SET */
    unsigned short *array;    /* массивы для GETALL, SETALL */
                              /* часть, особенная для Linux: */
    struct seminfo *__buf;    /* буфер для IPC_INFO */
} arg;
struct sembuf serv_sem_lock;      //блокировка ресурса
struct sembuf serv_sem_unlock;    //освобождение ресурса
struct sembuf client_wait_for_server;
struct sembuf client_sem_unlock;
struct sembuf client_sem_lock;

int rc;
int shm_id;
char *shm;
key_t key;
#define path_name "server"
#define proj_id 'A'

void sem_init();

#endif // SHARED_MEMORY_SYSTEM_V


#ifdef SHARED_MEMORY_POSIX

#define sem_name1 "/sem1"
#define sem_name2 "/sem2"
#define sem_name3 "/sem3"
#define shm_name "/shm"

sem_t *sid1, *sid2, *sid3 ;
int shm_id;

#endif // SHARED_MEMORY_POSIX


#endif // HEADERS_H