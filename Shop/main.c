/* 		Создать приложение, реализующee многопоточность. 
 * Создать концепцию магазина товаров.
 * Три потока покупателей и один поток поставщик товаров.
 * Пять ячеек памяти - товары. Поток поставщик заполняет доступную в данный момент времени ячейку товаром (~500 ед. в каждой    * ячейке) с интервалом 2 сек.
 * Потоки - покупатели забирают товар из доступных в данный момент ячеек памяти с частотой 1 сек (~2000 +-200 ед. нужно каждому  * покупателю).
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
	/* void *consumerStatus[NUM_CONSUMERS];
	memset(consumerStatus, 0, sizeof(consumerStatus));	
	void *producerStatus = NULL; */

	set_shared_data();	
	srand(time(NULL));

	/* Создание всех потребителей и поставщика */
	for (i = 0; i < NUM_CONSUMERS; i++)
		pthread_create(&tid_consumers[i], NULL, consumer_func, &consumers[i]);
	
	pthread_create(&tid_producer, NULL, producer_func, &producer);

	/* Ожидание завершения потребителей и поставщика */
	for (i = 0; i < NUM_CONSUMERS; i++)
	{
		pthread_join(tid_consumers[i], NULL);
		printf("Consumer #%lu gets %d volume of goods\n", tid_consumers[i], consumers[i]);						
	}
	pthread_join(tid_producer, NULL);
	printf("Producer #%lu delivered %d goods\n", tid_producer, producer);	
	return 0;
}