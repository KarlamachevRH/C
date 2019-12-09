/*  Тема лабораторной работы.
	Написать сервер и 2 типа клиента. На сервере работает синхронизированная очередь из N сообщений. Идет прием сообщений и отправка клиентам одновременно по TCP
протоколу.
	Если очередь не заполнена, то сервер каждые K секунд посылает UDP пакет "Жду сообщений" в локальную сеть. Если клиент 1-го типа получает этот пакет, то 
отправляет по TCP сообщение (время T обработки сообщения, длина случайной строки, случайная строка) . После отправки сообщения клиент засыпает на время T. Строка 
случайной длины и время T - генерируется случайно, максимальная длина строки и интервал времени задается через константы. Как только очередь заполняется, то сервер 
перестает слать UDP оповещение.
	Если в очереди есть сообщения, то сервер посылает каждые L секунд UDP пакет "есть сообщения" в локальную сеть. Клиент 2-го типа получив такой UDP пакет 
устанавливает соединение с сервером и получает по TCP от сервера сообщение со строкой. Обрабатывает T секунд(просто спит) и только после сможет опять получать 
сообщения. Как только очередь опусташается, то сервер перестает слать UDP оповещение.
	Кол-во клиентов 1-го и 2-го типа может быть неограниченно.
Сделать make-файл для сборки сервера и клиентов. Реализовать сервер и клиенты на языке С. Использовать компилятор gcc. Стандарт языка С99. Результаты работы 
разместить на сайте git-репозитория (bitbucket.org, github.com и т.д.). Пригласить преподавателя на сайте git-репозитория в качестве редактора ресурсов для проверки 
результатов. */

#include "interface.h"
#include <sys/ipc.h>
#include <sys/msg.h>  

#define SERV_QUEUE_SIZE 5
#define N 6
#define K 6
#define L 4

//структура данных для очереди сообщений
struct mymsgbuf {
	long mtype; //тип данных
	int T;
	int len;
	char mtext[MAX_SEND_SIZE];
};

// Функция обработки ошибок
void error(const char *msg){
    perror(msg);
    exit(1);
}

// глобальная переменная – количество
// активных пользователей 
int nclients = 0;

//Список функций:
//----------------------------------------------------------------------------------------------

//создать сокет с отправкой данных по TCP - протоколу
void create_socket_TCP(int *sockfd);

//связать сокет с определенным адресом
void bind_TCP_socket_to_local_addr(char **argv, int *sockfd, struct sockaddr_in *serv_addr, int portno);

//проверка типа поключившегося клиента (первого или второго типа)
int who_connected(int sock);

//запуск TCP - сервера
void tcp_server(char **argv, int portno, int msgqid, struct msqid_ds *info);

//получить данные от клиента 1
void recieve_data_from_client1(int sock, struct TCPmsgbuf *buff);

// получить буфер известного размера от клиента
int recieve_all_data(int sock, void *buff, int len);

//отправить данные в список сообщений IPC сервера
void send_message_to_queue(int qid, struct mymsgbuf *qbuf, long type, struct TCPmsgbuf *buff);

//принять данные из очереди сообщений
void read_message_from_queue(int qid, struct mymsgbuf *qbuf, long type);

//отправить данные клиенту 2
void send_data_to_client2(int sock, int msgqid);

// отправить весь буфер целиком (с проверкой полноты отправки данных)
int send_all_data(int sock, void *buff, int len);

// печать количества активных
// пользователей 
void printusers();

//----------------------------------------------------------------------------------------------

//создать сокет с отправкой данных по UDP - протоколу
void create_socket_UDP(int *sockfd);

//запуск UDP - сервера
void udp_server(char **argv,  int portno, int msgqid, struct msqid_ds *info);

//связать сокет с определенным адресом
void bind_UDP_socket_to_local_addr(char **argv, int *sockfd, struct sockaddr_in *addr, int portno);

//послать UDP - сообщение: "Жду сообщений!"
void waiting_msgs(int *sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *cli_addr);

//послать UDP - сообщение: "Есть сообщения в очереди!"
void msgs_in_queue(int sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *cli_addr);

//----------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
	int msgqid;     
	struct msqid_ds info;
	key_t key;
	key = ftok(".", 'm');
	
	int pid[PROCESS_NUM];

	printf("SERVER LW12\n");

	int TCPportno, UDPportno; // номер порта

	// ошибка в случае если мы не указали порт
    if (argc < 4) {
        fprintf(stderr,"Usage: %s <Host name> <TCP port number> <UDP port number>\n", argv[0]);
        exit(1);
    }
	TCPportno = atoi(argv[2]);
	UDPportno = atoi(argv[3]);
	
	//создать очередь сообщений
	if((msgqid = msgget(key, IPC_CREAT|0660)) == -1) {
		perror("msgget");
		exit(1);
	}   
	
	for (int i = 0; i < PROCESS_NUM; i++) {		
        // запускаем дочерний процесс 
        pid[i] = fork();

        if (-1 == pid[i]) {
            perror("fork"); /* произошла ошибка */
            exit(1); /*выход из родительского процесса*/

        } else if (0 == pid[i]) {
			//запуск tcp и udp серверов в отдельных процессах
			if(!i)
			udp_server(argv, UDPportno, msgqid, &info);
			else
			tcp_server(argv, TCPportno, msgqid, &info);	
			exit(EXIT_SUCCESS);		
		}		
	}	
	for (int i = 0; i < PROCESS_NUM; i++) {
		int stat;
        int status = waitpid(pid[i], &stat, 0);
        if (pid[i] == status) {
			if(!i)
            printf("Сервер UDP отключен, result=%d\n", WEXITSTATUS(stat));
			else
			printf("Сервер TCP отключен, result=%d\n", WEXITSTATUS(stat));
            fflush(stdout);
        }
    }	
	msgctl(msgqid, IPC_RMID, 0);
	return 0;
}

//запуск TCP - сервера
void tcp_server(char **argv, int portno, int msgqid, struct msqid_ds *info){
	int sockfd, newsockfd; // дескрипторы сокетов
	socklen_t clilen; // размер адреса клиента типа socklen_t
	struct sockaddr_in serv_addr, cli_addr;	//структуры с адресами сервера и клиента
	int pid; // id номер процесса	
	struct TCPmsgbuf buff;
	struct mymsgbuf qbuf;		

	int whoConnected = 3;

	char quitBuff[BUFF_SIZE] = {0};

	// Шаг 1 - создание сокета 
	create_socket_TCP(&sockfd);
	 
	// Шаг 2 - связывание сокетов с локальным адресом    
	bind_TCP_socket_to_local_addr(argv, &sockfd, &serv_addr, portno);

    // Шаг 3 - ожидание подключений, размер очереди - QUEUE_SIZE
	listen(sockfd, SERV_QUEUE_SIZE);
    clilen = sizeof(cli_addr);

	printf("TCP socket created.\n");
	
	// Шаг 4 - извлекаем сообщение из очереди
	// цикл извлечения запросов на подключение из очереди 
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
			int isExit = 0;
			while (!isExit){	
				
				whoConnected = who_connected(newsockfd);
				//если подключился клиент 1 типа, то получим от него данные и запишем в очередь сообщений сервера
				if(whoConnected == 1){

					printf("Подключен клиент 1 типа.\n");
					recieve_data_from_client1(newsockfd, &buff);
					printf("Recieved data from TCP socket.\n");
					send_message_to_queue(msgqid, &qbuf, 1, &buff);										

				}else if(whoConnected == 2){

					//отправим сообщение клиенту 2 типа
					printf("Подключен клиент 2 типа.\n");
					msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений

					if((unsigned long)info->msg_qnum == 0) 
					isExit = 1; //нечего отправлять, выходим					
					else {
						send_data_to_client2(newsockfd, msgqid);	
						printf("Sended data to TCP socket.\n");					
					}
				}else isExit = 1;
			}
			printf("-disconnect\n");
			nclients--;			
			exit(EXIT_SUCCESS);	
		}
		close(newsockfd);

		fgets(&quitBuff[0], sizeof(quitBuff) - 1, stdin);
		// проверка на "quit"
		if (!strcmp(&quitBuff[0], "quit\n")) {
			// Корректный выход
			printf("Exit...");
			close(sockfd);
			exit(0);
		}

    } /* end of while */
}

//создать сокет с отправкой данных по TCP - протоколу
void create_socket_TCP(int *sockfd){
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);	
    // ошибка при создании сокета
	if (*sockfd < 0) 
       error("ERROR opening socket");
}

//проверка типа поключившегося клиента (первого или второго типа)
int who_connected(int sock){	
	int bytes_recv = 0; // размер принятого сообщения
	int clientType = 0;
	bytes_recv = recieve_all_data(sock, &clientType, sizeof(int));
	if (bytes_recv <= 0) error("ERROR reading from socket");
	return clientType;
}

//получить данные от клиента 1
void recieve_data_from_client1(int sock, struct TCPmsgbuf *buff) {	
	int bytes_recv = 0; // размер принятого сообщения		
	int dataSizeRecv = 0; // размер получаемых данных
    
   	// обработка первого параметра (получение размера данных)
	bytes_recv = recieve_all_data(sock, &dataSizeRecv, sizeof(int));
	if (bytes_recv <= 0) error("ERROR reading from socket");

	printf("Получены данные:\nразмер структуры %d байт\n", dataSizeRecv);
	
	//получение структуры данных
	bytes_recv = recieve_all_data(sock, buff, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR reading from socket");		

	//получение размера строки
	bytes_recv = recieve_all_data(sock, &dataSizeRecv, sizeof(int));
	if (bytes_recv <= 0) error("ERROR reading from socket");

	buff->mtext = calloc(dataSizeRecv, sizeof(char));	
	if(!buff->mtext) {
		printf("Memory allocation error!");
		exit (1);
	}

	//получение строки
	bytes_recv = recieve_all_data(sock, buff->mtext, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR reading from socket");	

	printf("Получены данные:\nдлина строки %d байт. Строка:\n%s\n", buff->len, buff->mtext);
}

//отправить данные клиенту 2
void send_data_to_client2(int sock, int msgqid){
	int bytes_send = 0; // размер отправленного сообщения		
	int dataSizeSend = 0; // размер отправляемых данных		
	struct mymsgbuf queueBuf;	//буфер для полученных данных из очереди сообщений
	struct TCPmsgbuf bufForSend; //буфер для отправки клиенту
	
	read_message_from_queue(msgqid, &queueBuf, 0); //получим сообщение из очереди	

	//копируем данные
	bufForSend.t = queueBuf.T;
	bufForSend.len = queueBuf.len;
	
	//скопируем указатель в буфер для отправки клиенту 2
	bufForSend.mtext = calloc(bufForSend.len, sizeof(char));
	strcpy(bufForSend.mtext, queueBuf.mtext);		
	
	dataSizeSend = sizeof(struct TCPmsgbuf);
	bytes_send = send_all_data(sock, &dataSizeSend, sizeof(int)); //отправка размера структуры данных клиенту
	if (bytes_send < 0) error("ERROR send data to socket");	
	
	bytes_send = send_all_data(sock, &bufForSend, dataSizeSend); 
	if (bytes_send < 0) error("ERROR send data to socket"); 

	dataSizeSend = bufForSend.len;
	bytes_send = send_all_data(sock, &dataSizeSend, sizeof(int)); //отправка размера строки клиенту
	if (bytes_send < 0) error("ERROR send data to socket");	
	
	bytes_send = send_all_data(sock, bufForSend.mtext, dataSizeSend); //отправка строки клиенту
	if (bytes_send < 0) error("ERROR send data to socket");

	if(bufForSend.mtext != NULL) free(bufForSend.mtext);
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
    return total;
}

//отправить данные в список сообщений IPC сервера
void send_message_to_queue(int qid, struct mymsgbuf *qbuf, long type, struct TCPmsgbuf *buff){
	int length = 0;	

	qbuf->mtype = type;
	qbuf->T = buff->t;
	qbuf->len = buff->len;
	
	strcpy(qbuf->mtext, buff->mtext);
	
	length = sizeof(struct mymsgbuf) - sizeof(long);

	//отправим сообщение
	if((msgsnd(qid, qbuf, length, 0)) ==-1){
			perror("msgsnd");
			exit(1);
	}
	printf("Data sended to queue...\n");
}

//принять данные из списка сообщений IPC сервера
void read_message_from_queue(int qid, struct mymsgbuf *qbuf, long type){
	
	int msgSize = sizeof(struct mymsgbuf) - sizeof(long);	
	qbuf->mtype = type;
	msgrcv(qid, qbuf, msgSize, type, 0);       
	printf("MESSAGE RECEIVED from queue...\n");
}

//запуск UDP - сервера
void udp_server(char **argv, int portno, int msgqid, struct msqid_ds *info){
	int sockfd; // дескриптор сокета	
	struct sockaddr_in local_addr;	//структура с локальным адресом 
	int pid[PROCESS_NUM]; // id номера процессов		

	// Шаг 1 - создание сокета 
	create_socket_UDP(&sockfd);
	 
	// Шаг 2 - связывание сокетов с локальным адресом    
	bind_UDP_socket_to_local_addr(argv, &sockfd, &local_addr, portno);    

	printf("UDP socket created.\n");
	
	// Шаг 3 - получение количества сообщений в IPC - очереди сообщений на сервере и
	// отправка пакетов с запросом данных/готовностью к отправке данных в сеть
	for (int i = 0; i < PROCESS_NUM; i++) { 
		pid[i] = fork();
        if (pid[i] < 0)
            error("ERROR on fork");
        if (pid[i] == 0) {

			if(!i){

				while(1)
					waiting_msgs(&sockfd, msgqid, info, &local_addr);	

			}else {

				while(1) 
					msgs_in_queue(sockfd, msgqid, info, &local_addr);	

			}
		}
	}
    close(sockfd);	   
}

//послать UDP - сообщение: "Жду сообщений!"
void waiting_msgs(int *sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *local_addr){
	int n;
	int isExit = 0;		
		
	socklen_t size = sizeof(struct sockaddr_in);

	char quitBuff[BUFF_SIZE] = {0};

	while (!isExit){
		sleep(K);
		msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений
		if((unsigned long)info->msg_qnum >= (unsigned long)N) isExit = 1;
		else{			
			n = sendto(*sockfd, msgToRecieve, strlen(msgToRecieve), 0, (struct sockaddr *)local_addr, size);
			if (n < 0) error("sendto");
			printf("Sended \"Waiting data\" message to UDP socket.\n");	
		}			
	}
	printf("Enter \"quit\" for exit...\n");
	fgets(&quitBuff[0], sizeof(quitBuff) - 1, stdin);
	// проверка на "quit"
	if (!strcmp(&quitBuff[0], "quit\n")) {
		// Корректный выход
		printf("Exit...\n");
		close(*sockfd);
		exit(0);
	}				
}

//послать UDP - сообщение: "Есть сообщения в очереди!"
void msgs_in_queue(int sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *cli_addr){
	int n;
	int isExit = 0;		
	socklen_t size = sizeof(struct sockaddr_in);

	char quitBuff[BUFF_SIZE] = {0};
			
	while (!isExit){
		sleep(L);
		msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений
		if((unsigned long)info->msg_qnum == 0) isExit = 1;
		else{			
			n = sendto(sockfd, msgToSend, strlen(msgToSend), 0,(struct sockaddr *)cli_addr, size);
			if (n < 0) error("sendto");
			printf("Sended \"Have data for sending\" message to UDP socket.\n");
		}			
	}
	printf("Enter \"quit\" for exit...\n");
	fgets(&quitBuff[0], sizeof(quitBuff) - 1, stdin);
	// проверка на "quit"
	if (!strcmp(&quitBuff[0], "quit\n")) {
		// Корректный выход
		printf("Exit...\n");
		close(sockfd);
		exit(0);
	}				
}

//создать сокет с отправкой данных по UDP - протоколу
void create_socket_UDP(int *sockfd){
	*sockfd = socket(AF_INET, SOCK_DGRAM, 0);	
    // ошибка при создании сокета
	if (*sockfd < 0) 
       error("ERROR opening socket");
}

//связать сокет с определенным адресом
void bind_UDP_socket_to_local_addr(char **argv, int *sockfd, struct sockaddr_in *addr, int portno){
	struct hostent *hp; // структура хоста
	memset(addr, 0, sizeof(struct sockaddr_in));	
    addr->sin_family = AF_INET;
	hp = gethostbyname(argv[1]); // извлечение хоста
	if (hp==0) error("Unknown host");
	memmove(&addr->sin_addr.s_addr, hp->h_addr, hp->h_length);    
    addr->sin_port = htons(portno);    
}

//связать сокет с определенным адресом
void bind_TCP_socket_to_local_addr(char **argv, int *sockfd, struct sockaddr_in *addr, int portno){
	struct hostent *hp; // структура хоста
	memset(addr, 0, sizeof(struct sockaddr_in));	
    addr->sin_family = AF_INET;
	hp = gethostbyname(argv[1]); // извлечение хоста
	if (hp==0) error("Unknown host");
	memmove(&addr->sin_addr.s_addr, hp->h_addr, hp->h_length);    
    addr->sin_port = htons(portno);
    // вызываем bind для связывания
	if (bind(*sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0) 
            error("ERROR on binding");
}

// печать количества активных
// пользователей 
void printusers(){ 
	if(nclients)
	{printf("%d user on-line\n", nclients);}
	else {printf("No user on line\n");}
}
