/* 		Создать приложение, реализующee многопоточность. 
 * Создать концепцию магазина товаров.
 * Три потока покупателей и один поток поставщик товаров.
 * Пять ячеек памяти - товары. Поток поставщик заполняет доступную в данный момент
 * времени ячейку товаром (~500 ед. в каждой ячейке) с интервалом 2 сек.
 * Потоки - покупатели забирают товар из доступных в данный момент ячеек памяти с 
 * частотой 1 сек (~2000 +-200 ед. нужно каждому покупателю).
 * Покупатели берут товар по ~300 +- 50 ед.
 * Создать массив mutex - блокировок для доступа к ячейкам памяти.
 */

#include "pthread_create.h"

int main(int argc, char **argv)
{   
	int	consumers[NUM_CONSUMERS];
	memset(consumers, 0, sizeof(consumers));
	int producer = 0;
	pthread_t tid_producer, tid_consumers[NUM_CONSUMERS];
	int i;
	int *consumerStatus[NUM_CONSUMERS];
	memset(consumerStatus, 0, sizeof(consumerStatus));
	int *producerStatus = NULL;

	set_shared_data();
	srand(time(NULL));

	/* Создание всех потребителей и поставщика */
	for (i = 0; i < NUM_CONSUMERS; i++)
		pthread_create(&tid_consumers[i], NULL, consumer_func, &consumers[i]);
	
	pthread_create(&tid_producer, NULL, producer_func, &producer);

	/* Ожидание завершения потребителей и поставщика */
	for (i = 0; i < NUM_CONSUMERS; i++)
	{
		pthread_join(tid_consumers[i], (void**)&consumerStatus[i]);
		if(*(int*)consumerStatus[i] == 0)
			printf("Consumer #%lu gets %d volume of goods\n", tid_consumers[i], consumers[i]);
		else
		{
			printf("Producer's goods ended\n");
			printf("Consumer #%lu gets %d volume of goods\n", tid_consumers[i], consumers[i]);
		}
	}
	pthread_join(tid_producer, (void**)&producerStatus);
	if(*(int*)producerStatus == 0)
		printf("Producer #%lu delivered goods %d time\n", tid_producer, producer);

	for(i = 0; i < NUM_CONSUMERS; i++)
	{
		if(consumerStatus[i])
			free(consumerStatus[i]);
	}
	if(producerStatus)
		free(producerStatus);
	return 0;
}