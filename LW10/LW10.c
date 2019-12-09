/*  МЕТОДИЧЕСКИЕ УКАЗАНИЯ. 
1. Для уточнения списка заголовочных файлов, необходимых для вызова функций библиотеки pthread используйте инструкции man и info.
2. Хотя функции работы с потоками описаны в файле включения pthread.h, на самом деле они находятся в библиотеке. Поэтому процесс компиляции и сборки многопоточной программы выполняется в два этапа, например:
gcc -Wall -c -o test.o test.c
gcc -Wall -o test test.o <path>libgcc.a –lpthread
3. Библиотеку libgcc.a рекомендуется скопировать в текущий каталог.
4. Для поиска библиотеки средствами файлового менеджера Midnight Commander используйте сочетание клавиш <Alt> - <Shift> - ?
5. Для просмотра результата выполнения программы используйте сочетание клавиш <Ctrl> - O. Они работают и в режиме редактирования файла.
6. Для протоколирования результатов выполнения программ целесообразно использовать перенаправление вывода с консоли в файл: ./test > result.txt
7. Для доступа к файлам на сервере Linux, применяйте протокол ftp, клиентская программа которого имеется в Windows и встроена в файловые менеджеры FAR и TotalCommander. При этом учетная запись и пароль те же, что и при подключении по протоколу ssh.
    ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ. 
1. Для вариантов заданий написать и отладить программу, реализующую родительский процесс, вызывающий и отслеживающий состояние порожденных потоков.
2. Добавить в написанную программу синхронизацию обращения потоков к какому-либо общему ресурсу, используя взаимные исключения.
    ВАРИАНТ ЗАДАНИЯ.
5. Винни-Пух и пчелы. Заданное количество пчел добывают мед равными порциями, задерживаясь в пути на случайное время. Винни-Пух потребляет мед порциями заданной величины за заданное время и столько же времени может прожить без питания. Работа каждой пчелы реализуется в порожденном процессе. */

#define _XOPEN_SOURCE
#include <pthread.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <wait.h> 
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define	MAX_HONEY_VOLUME 1000
#define	MAXNBEES 100
#define	BEE_HONEY_PORTION 10
#define	POOH_HONEY_PORTION 200
#define	INIT_HONEY 400
#define	min(a,b)	((a) < (b) ? (a) : (b))

struct {
	pthread_mutex_t	mutex;
	int	honey;
	int poohStatus;	
} shared = { 
	PTHREAD_MUTEX_INITIALIZER,
    INIT_HONEY
};

void *bee(void*), *pooh(void*);

int main(int argc, char **argv){
	int i, nBees, producer[MAXNBEES];
	void *producerStatus[MAXNBEES];       
	pthread_t tid_bee[MAXNBEES], tid_pooh;
	int consumer = 0;
	void *consumerStatus = malloc(sizeof(int));		
	if (argc < 2) {
		printf("Usage: %s <number of bees(digit)>", argv[0]);
		exit(-1);
	}	
	nBees = min(atoi(argv[1]), MAXNBEES);
	/* создание всех пчел и Винни */	
	for (i = 0; i < nBees; i++) {	
		producer[i] = 0;		
		pthread_create(&tid_bee[i], NULL, bee, &producer[i]);		
	}
	shared.poohStatus = 0;
	pthread_create(&tid_pooh, NULL, pooh, &consumer);
	/* ожидание завершения потребителя и производителей */ 
	pthread_join(tid_pooh, consumerStatus);
	if(*(int*)consumerStatus == -1) 
	printf("Pooh died of hunger!\n");
	else if(*(int*)consumerStatus == 0)	
	printf("Pooh #%lu eat %d portions of honey. Pooh not hungry.\n", tid_pooh, consumer);	
	free(consumerStatus);
	for (i = 0; i < nBees; i++) {			
		pthread_join(tid_bee[i], &producerStatus[i]);		
		if((int*)producerStatus[i] == 0){
			printf("Honey volume is full.\n");			
			printf("Bee #%lu brought %d portions of honey\n", tid_bee[i], producer[i]);
		} else {
			printf("No Pooh. Honey is not needed. End programm.\n");
			printf("Bee #%lu brought %d portions of honey\n", tid_bee[i], producer[i]);
		}					
	}	
	exit(0);
}

void *bee(void *arg){
	int honeyFullCnt = 0;	
	srand(pthread_self());	
	for ( ; ; ) {		
        sleep(rand()%10);
		pthread_mutex_lock(&shared.mutex);
		if(shared.poohStatus == -1){
			pthread_mutex_unlock(&shared.mutex);
			pthread_exit((void*)-1);
		}				
		else {
			if (shared.honey < MAX_HONEY_VOLUME) {            
			shared.honey +=	BEE_HONEY_PORTION;
			*((int*)arg) += 1;	
			printf("Bee #%lu brought portions of honey. Honey volume %d\n", pthread_self(), shared.honey);	
			pthread_mutex_unlock(&shared.mutex);		
		}
			else honeyFullCnt++;		
			pthread_mutex_unlock(&shared.mutex);		
			if(honeyFullCnt > 10) 
			pthread_exit((void*)0);
		}				
	}
}
void *pooh(void *arg){
	int honeyFullCnt = 0;	
	for ( ; ; ) {
        sleep(2);
        pthread_mutex_lock(&shared.mutex);
        if(shared.honey/POOH_HONEY_PORTION > 0){
			shared.honey -= POOH_HONEY_PORTION;
			printf("Pooh #%lu eat portions of honey. Honey volume %d\n", pthread_self(), shared.honey);
			*((int*)arg) += 1;
		}        
        else {
            pthread_mutex_unlock(&shared.mutex);   
			shared.poohStatus = -1;	         
            pthread_exit((void*)-1);
        }
        pthread_mutex_unlock(&shared.mutex);
		if (shared.honey >= (MAX_HONEY_VOLUME - POOH_HONEY_PORTION)) 
		honeyFullCnt++;
		if(honeyFullCnt > 2) {	
			shared.poohStatus = -1;		
			pthread_exit((void*)0);		
		}
		sleep(2);		
	}	
}