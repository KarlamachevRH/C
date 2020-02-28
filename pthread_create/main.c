#define _XOPEN_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* func(void *arg)
{
	int *ptr;
	ptr = (int*)arg;
	printf("%d\n", *ptr);
	pthread_exit(NULL);
}

int main(void) 
{
	int i, index[3], *p;
	pthread_t tid;

	p = index;

	for(i = 1; i < 4; i++, p++)
	{
		*p = i;
		pthread_create(&tid, NULL, func, p);
	}
	sleep(1);
	return 0;
}