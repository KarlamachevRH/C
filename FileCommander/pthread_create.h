#ifndef __PTHREAD_H__
#define __PTHREAD_H__
#define _GNU_SOURCE

#include <pthread.h>
#include <fcntl.h>
#include <wait.h> 
#include "cursed_menu.h"

#define BUFF_SIZE 255 /* Размер буфера для копирования данных (файлов) */
#define BYTES_DONE 0

/* Структура для предачи данных в функцию для создания потока копирования файлов */
struct copyData
{
    char pathFrom[PATH_MAX];    
    char pathTo[PATH_MAX];
};

/* 
 * Структура для предачи данных в функцию для создания потока 
 * отображения прогресса копирования 
 */
struct progressBarData
{
    cursed_window **panels;    
    struct copyData data;
    double progress;
};

unsigned long long calculate_size(char *path);
void copy(char *pathFrom, char *pathTo);
void* copy_thread(void *data);
void* progress_bar(void *data);
int create_threads(char *pathFrom, char *pathTo, cursed_window **windows);

#endif // __PTHREAD_H__