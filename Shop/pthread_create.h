#ifndef __PTHREAD_CREATE_H__
#define __PTHREAD_CREATE_H__
#define _XOPEN_SOURCE
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define PRODUCTS_NUM 5
#define	PRODUCER_MAX_DELIVERY_ITERATIONS 50
#define	PRODUCT_VOLUME 500
#define	NUM_CONSUMERS 3
#define	PRODUCT_CONSUMPTION_VOL 300
#define	CONSUMER_PRODUCT_VOL 2000
#define	INIT_PRODUCTS 0

struct shared_data
{
	pthread_mutex_t	mutex[PRODUCTS_NUM + 1];
	int	products[PRODUCTS_NUM];
	int producerStatus;
	int boughtConsumersCounter;
} shared;

void set_shared_data();
int generate_rand_num(int num);
void *producer_func(void *arg);
void *consumer_func(void *arg);

#endif // __PTHREAD_CREATE_H__