/*   МЕТОДИЧЕСКИЕ УКАЗАНИЯ. 
1. Для отладки приложения рекомендуется запускать клиент и сервер на одной машине.
2. Для отладки рекомендуется использовать отладчик gdb, информацию котором можно получить инструкцией man gdb.
3. Выбор протокола передачи данных определяется вариантом задания.
4. Для нормального выполнения сетевых соединений между компьютерами требуется соответствующая настройка межсетевых экранов.
	ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ. 
1. Написать и отладить функцию, реализующую сервер, получающий и обрабатывающий запросы от клиентов (аналог родительского приложения).
2. Написать и отладить программу, реализующую клиентский процесс (аналог дочернего процесса), запрашивающий у сервера исходные данные, выполняющий вычисления (действия) в соответствии с вариантом, и возвращающий серверу результаты вычислений.
	ВАРИАНТЫ ЗАДАНИЙ. 
См. варианты заданий в файле variants.txt.
Для нечетных вариантов используется протокол TCP, для четных – UDP.
    ВАРИАНТ ЗАДАНИЯ.
10. Авиаразведка. Создается условная карта в виде матрицы, размерность которой определяет размер карты, содержащей произвольное количество единиц (целей) в произвольных ячейках. Из произвольной точки карты стартуют несколько разведчиков (процессов), курсы которых выбираются так, чтобы покрыть максимальную площадь карты. Каждый разведчик фиксирует цели, чьи координаты совпадают с его координатами и по достижении границ карты сообщает количество обнаруженных целей. */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>  

#define BUFF_MAP 100000
#define FNAME_MAX 60  
#define NUM_OF_VAR 4 //количество переменных в файле данных
#define QUEUE_SIZE 10
#define INIT_KEY 77

//используем семафоры для доступа процессов к разделяемой памяти
union semun {
 int val;                  /* значение для SETVAL */
 struct semid_ds *buf;     /* буферы для  IPC_STAT, IPC_SET */
 unsigned short *array;    /* массивы для GETALL, SETALL */
                           /* часть, особенная для Linux: */
 struct seminfo *__buf;    /* буфер для IPC_INFO */
}; 

// Функция обработки ошибок
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// глобальная переменная – количество
// активных пользователей 
int nclients = 0;

// координаты случайных целей
struct targetsCoordinates {
	int targetX;
	int targetY;
};

// макрос для печати количества активных
// пользователей 
void printusers();
// отправить весь буфер целиком
int send_all_data(int sock, void *buff, int len);
// получить буфер известного размера
int recieve_all_data(int sock, void *buff, int len);
// read file with required data - port number, map size and number of targets on map
void read_file(char *argv, FILE *fd, int *portno, int *mapX, int *mapY, int *numOfTargets);
// получение случайных целей
void generate_targets(struct targetsCoordinates *targets, int numOfTargets, int mapX, int mapY);
// инициализация данных карты
void map_initialization(int **map, struct targetsCoordinates *targets, int numOfTargets, int mapX, int mapY);
//вывод координат найденных разведчиками целей
void show_targets(struct targetsCoordinates *targets, int numOfTargets);
//отправка карты разведчикам
void send_map(int sock, int **map, int mapX, int mapY);
//получить результаты поиска разведчиками целей на карте
void recieve_results(int sock, struct targetsCoordinates *targetsRecievedFromClients);

int main(int argc, char *argv[]) {    	
	FILE *fd = NULL; //указатель файла служебных данных
	printf("TCP SERVER DEMO\n");
	
	int sockfd, newsockfd; // дескрипторы сокетов
	int portno; // номер порта
	int pid; // id номер потока
    socklen_t clilen; // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента

	int mapX = 0, mapY = 0; //размеры карты авиаразведки	
	int numOfTargets = 0; //количество целей

	int shmid;
	key_t key = INIT_KEY;
	int *isMapSended;

	int semid;
	union semun arg;
	struct sembuf lock_res = {0, -1, 0};  //блокировка ресурса
	struct sembuf rel_res = {0, 1, 0};	//освобождение ресурса

	/* Получим ключ. Один и тот же ключ можно использовать как
	для семафора, так и для разделяемой памяти */
	if((key = ftok(".", 'S')) < 0){
		printf("Невозможно получить ключ\n");
		exit(1);
	}

	/* Создадим семафор - для синхронизации работы с разделяемой памятью.*/
	semid = semget(key, 1, 0666 | IPC_CREAT);

	/* Установить в семафоре № 0 (Контроллер ресурса) значение "1" */
	arg.val = 1;
	semctl(semid, 0, SETVAL, arg);
		
	/* Создадим область разделяемой памяти */
	if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	/* Получим доступ к разделяемой памяти */
	if ((isMapSended = (int*)shmat(shmid, NULL, 0)) == (int*)-1) {
		perror("shmat");
		exit(1);
	}	

	// ошибка в случае если мы не указали порт
    if (argc < 2) {
        fprintf(stderr,"Usage: %s <data file name>\n", argv[0]);
        exit(1);
    }
	read_file(argv[1], fd, &portno, &mapX, &mapY, &numOfTargets);	//чтение данных из файла

	int **map = (int**)calloc(mapX, sizeof(int*)); //карта авиаразведки
	if (map==NULL) {
		printf("Memory allocation error!\n");
		exit(EXIT_FAILURE);
	}
	for(int i = 0; i<mapX; i++){
		map[i] = (int*)calloc(mapY, sizeof(int));
		if (map[i]==NULL) {
			printf("Memory allocation error!\n");
			free(map);
			exit(EXIT_FAILURE);
		}
	}

	struct targetsCoordinates targets[numOfTargets]; //массив для координат целей
	memset(targets, 0, sizeof(targets));	

	generate_targets(targets, numOfTargets, mapX, mapY); //получение случайных целей
	printf("Цели, сгенерированые на сервере:\n");
	show_targets(targets, numOfTargets);

	map_initialization(map, targets, numOfTargets, mapX, mapY); //инициализация данных карты с координатами целей	
	
    // Шаг 1 - создание сокета	    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // ошибка при создании сокета
	if (sockfd < 0) 
       error("ERROR opening socket");
	 
	// Шаг 2 - связывание сокета с локальным адресом    
	memset(&serv_addr, 0, sizeof(serv_addr));    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // сервер принимает подключения на все адреса сетевых подключений сервера
    serv_addr.sin_port = htons(portno);
    // вызываем bind для связывания
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    // Шаг 3 - ожидание подключений, размер очереди - QUEUE_SIZE
	listen(sockfd, QUEUE_SIZE);
    clilen = sizeof(cli_addr);
	
	// Шаг 4 - извлекаем сообщение из очереди
	// цикл извлечения запросов на подключение из очереди   	
	*isMapSended = 0;
	while (1) {
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
		nclients++; // увеличиваем счетчик подключившихся клиентов              			
		printusers();
		
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0) {			
            close(sockfd);

			/* Получим доступ к разделяемой памяти */
			if ((isMapSended = (int*)shmat(shmid, NULL, 0)) == (int*)-1) {
				perror("shmat");
				printf("Ошибка доступа к разделяемой памяти\n");
				exit(1);
			}

			/* Заблокируем разделяемую память */	
			if((semop(semid, &lock_res, 1)) == -1){
				fprintf(stderr, "Lock failed\n");
				exit(1);
			} else{
				printf("Semaphore resources decremented by one (locked)\n");
				fflush(stdout);
			}

			if(*isMapSended == 0){
				send_map(newsockfd, map, mapX, mapY); //отправка карты 1 клиенту(род. процессу клиентов)
				*isMapSended = 1;
				printf("Карта отправлена\n");
				for(int i = 0; i<mapX; i++){
					if(map[i] != NULL)free(map[i]);
				}
				free(map);				
			} else{				
				struct targetsCoordinates buff[numOfTargets];
				memset(buff, 0, sizeof(buff));				
				recieve_results(newsockfd, buff); //получение результатов от клиента

				printf("Цели, полученные от разведчика #%d: \n", --nclients);				
				show_targets(buff, numOfTargets); //вывод данных

				for(int i = 0; i<mapX; i++){
					if(map[i] != NULL)free(map[i]);
				}
				free(map);				
			}
			/* Освободим разделяемую память */
			if (shmdt(isMapSended) < 0) {
			printf("Ошибка отключения\n");
			exit(1);
			}			
			if((semop(semid, &rel_res, 1)) == -1){
					fprintf(stderr, "Unlock failed\n");
					exit(1);
			} else{
				printf("Semaphore resources incremented by one (unlocked)\n");
				fflush(stdout);
			}	
			exit(EXIT_SUCCESS);	
        } else close(newsockfd);		
    } /* end of while */
    close(sockfd);
	for(int i = 0; i<mapX; i++){
		if(map[i] != NULL)free(map[i]);
	}
	free(map);
    return 0; 
}

// макрос для печати количества активных
// пользователей 
void printusers()
{ 
	if(nclients)
	{printf("%d user on-line\n", nclients);}
	else {printf("No user on line\n");}
}

// отправить весь буфер целиком
int send_all_data(int sock, void *buff, int len) {
    int total = 0;
    int n = 0;

    while(total < len && n >= 0) {
        n = send(sock, buff+total, len-total, 0);        
        total += n;
    }
    return (n==-1 ? -1 : total);
}

// получить буфер известного размера
int recieve_all_data(int sock, void *buff, int len){
	int total = 0;
    int n = 0;

    while(total < len && n >= 0) {		
        n = recv(sock, buff+total, len-total, 0);        
        total += n;
    }	
    return (n == 0 ? 0 : total);
}

// read file with required data - port number, map size and number of targets on map
void read_file(char *argv, FILE *fd, int *portno, int *mapX, int *mapY, int *numOfTargets) {	
	int i = 0;
	int buff[NUM_OF_VAR];
	if((fd = fopen(argv, "r")) == NULL) {
		printf("Data file open error!\n");
		exit(EXIT_FAILURE);
	}
	while((fscanf(fd, "%d", &buff[i])) != EOF){			
		i++;
	}
	if(i < NUM_OF_VAR){
		printf("Wrong data in service file!\n");
		exit(EXIT_FAILURE);
	} else {
		*portno = buff[0];
		*mapX = buff[1];
		*mapY = buff[2];
		*numOfTargets = buff[3];
	}	
}

// получение случайных целей
void generate_targets(struct targetsCoordinates *targets, int numOfTargets, int mapX, int mapY){	
	for(int i = 0; i < numOfTargets; i++){		
		srand ( time(NULL) + i);
		targets[i].targetX = rand()%mapX;
		targets[i].targetY = rand()%mapY;
	}	
}

// инициализация данных карты
void map_initialization(int **map, struct targetsCoordinates *targets, int numOfTargets, int mapX, int mapY){
	for(int i = 0; i < mapX; i++){
		for(int j = 0; j < mapY; j++){
			//перебираем массив структур с данными целей для инициализации соответствующих координат карты
			int isTarget = 0;			
			for(int k = 0; k < numOfTargets; k++){
				if(i == targets[k].targetX && j == targets[k].targetY)
				isTarget = 1;				
			}		 
			if(isTarget){
				map[i][j] = 1;
				//printf("map_initialization: map[%d][%d]\n", i, j);
			} else 
			map[i][j] = 0;
		}
	}	
}

void show_targets(struct targetsCoordinates *targets, int numOfTargets){	
	for(int i = 0; (targets[i].targetX != 0 || targets[i].targetY != 0) && i < numOfTargets; i++){
		printf("Target #%d: x = %d, y = %d\n", i, targets[i].targetX, targets[i].targetY);
	}
}

void send_map(int sock, int **map, int mapX, int mapY){
	int bytes_send = 0; // размер отправленного сообщения		
	int dataSizeSend = 0; // размер отправляемых данных			
	int mapSize[2] = {mapX, mapY};
	
	bytes_send = send_all_data(sock, mapSize, sizeof(mapSize)); //отправка размерности карты клиенту
	if (bytes_send < 0) error("ERROR send data to socket");	

	dataSizeSend = mapY*sizeof(int);
	bytes_send = send_all_data(sock, &dataSizeSend, sizeof(int)); //отправка размера блока данных клиенту
	if (bytes_send < 0) error("ERROR send data to socket");	
	for(int i = 0; i < mapX; i++){
		bytes_send = send_all_data(sock, map[i], dataSizeSend); //отправка карты с отмеченными целями клиенту
		if (bytes_send < 0) error("ERROR send data to socket");  
	}
		
	nclients--; // уменьшаем счетчик активных клиентов
	printf("-disconnect\n"); 
	printusers();
	return;
}

void recieve_results(int sock, struct targetsCoordinates *targetsRecievedFromClients) {	
	int bytes_recv = 0; // размер принятого сообщения		
	int dataSizeRecv = 0; // размер получаемых данных		
    
   	// обработка первого параметра (получение размера данных)
	bytes_recv = recieve_all_data(sock, &dataSizeRecv, sizeof(int));
	if (bytes_recv <= 0) error("ERROR reading from socket");
	
	bytes_recv = recieve_all_data(sock, targetsRecievedFromClients, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR reading from socket");			
	
	printf("-disconnect\n"); 
	printusers();
	return;
}