#include "pthread_create.h"

void set_shared_data()
{
    pthread_mutex_t initial_mutex = PTHREAD_MUTEX_INITIALIZER;
    for(int i = 0; i < PRODUCTS_NUM + 1; i++)
        memcpy(&shared.mutex[i], &initial_mutex, sizeof(initial_mutex));        
    for(int i = 0; i < PRODUCTS_NUM; i++)
        shared.products[i] = INIT_PRODUCTS;
    shared.producerStatus = 0;    
    shared.boughtConsumersCounter = 0;
}

int generate_rand_num(int num) // range: [-num, +num]
{
    int rangeForRandFunc = 0;
    int randNum = 0;
    rangeForRandFunc = num*2;
    randNum = rand()%(rangeForRandFunc + 1);
    if(randNum < num)
        randNum = 0 - (rangeForRandFunc - randNum);    
    else
        randNum = num - (num*2  - randNum);
    return randNum;
}

void *producer_func(void *arg)
{
	int cnt = 0;
	int consumersStatus = -1;
	while(1) 
	{
		sleep(2);        
		for(int i = 0; i < PRODUCTS_NUM; i++)
		{        
			if((pthread_mutex_trylock(&shared.mutex[i])) == 0)
			{
				shared.products[i] = PRODUCT_VOLUME;			
				printf("Producer tid%lu delivered goods. Product #%d\n", pthread_self(), i);
				pthread_mutex_unlock(&shared.mutex[i]);
				cnt++;
                *((int*)arg) = cnt;                           
			}
		}
		pthread_mutex_lock(&shared.mutex[PRODUCTS_NUM]);
		if(shared.boughtConsumersCounter == NUM_CONSUMERS)
			consumersStatus = 0;
		pthread_mutex_unlock(&shared.mutex[PRODUCTS_NUM]);
		if(cnt > PRODUCER_MAX_DELIVERY_ITERATIONS || consumersStatus == 0)
		{
			shared.producerStatus = -1;
			printf("Producer tid%lu ended work. %d deliveries made\n", pthread_self(), cnt);
			pthread_exit(NULL);			
		}
	}
}

void *consumer_func(void *arg)
{
    int cnt = 0;
	int consumptionVol = 0;
	int maxConsumptionVol = CONSUMER_PRODUCT_VOL + generate_rand_num(200);
	int goodsVolume = 0;
	while(goodsVolume < maxConsumptionVol) 
	{
        sleep(1);
		for(int i = 0; i < PRODUCTS_NUM && goodsVolume < maxConsumptionVol; i++)
		{
			if((pthread_mutex_trylock(&shared.mutex[i])) == 0)
			{
				pthread_mutex_lock(&shared.mutex[PRODUCTS_NUM]);
				if(shared.producerStatus == -1)
				{
                    *((int*)arg) = cnt;
					pthread_mutex_unlock(&shared.mutex[i]);
					pthread_mutex_unlock(&shared.mutex[PRODUCTS_NUM]);
					pthread_exit((void*)-1);
				}
				pthread_mutex_unlock(&shared.mutex[PRODUCTS_NUM]);
				consumptionVol = PRODUCT_CONSUMPTION_VOL + generate_rand_num(50);
				shared.products[i] -= consumptionVol;
				printf("Consumer tid%lu took away %d goods. Product #%d\n", pthread_self(), consumptionVol, i);
				pthread_mutex_unlock(&shared.mutex[i]);
				goodsVolume += consumptionVol;
                cnt++;
			}
		}
	}
    *((int*)arg) = cnt;
	pthread_mutex_lock(&shared.mutex[PRODUCTS_NUM]);
    shared.boughtConsumersCounter++;
	pthread_mutex_unlock(&shared.mutex[PRODUCTS_NUM]);
	printf("Consumer tid%lu ended. Bought %d goods\n", pthread_self(), goodsVolume);
	pthread_exit(NULL);
}
