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
результатов. 

	Модифицировать клиент-серверное приложение с использованием для сериализации  сообщений библиотеки protobuf-c. https://github.com/protobuf-c/protobuf-c
Создать на сайте git-репозитория дополнительную ветку protobuf и разместить свои результаты для проверки */

#include "interface.h"
#include <sys/ipc.h>
#include <sys/msg.h>  

#define SERV_QUEUE_SIZE 5
#define N 4
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
AMessage *recieve_data_from_client1(int sock);

// получить буфер известного размера от клиента
int recieve_all_data(int sock, void *buff, int len);

//отправить данные в список сообщений IPC сервера
void send_message_to_queue(int qid, struct mymsgbuf *qbuf, long type, AMessage *msg);

//принять данные из очереди сообщений
void read_message_from_queue(int qid, struct mymsgbuf *qbuf, long type);

//отправить данные клиенту 2
void send_data_to_client2(int sock, int msgqid, AMessage *msg);

// отправить весь буфер целиком (с проверкой полноты отправки данных)
int send_all_data(int sock, void *buff, int len);

// печать количества активных
// пользователей 
void printusers();

//----------------------------------------------------------------------------------------------

//создать сокет с отправкой данных по UDP - протоколу
void create_socket_UDP(int *sockfd);

//запуск UDP - сервера
void udp_server(char **argv, int portno1, int portno2, int msgqid, struct msqid_ds *info);

//связать сокет с определенным адресом
void bind_UDP_socket_to_local_addr(char **argv, int *UDPsock, struct sockaddr_in *addr, int portno);

// поиск в сетевых интерфейсах локального компьютера интерфейса,
// указанного в параметрах командной строки с присвоением широковещательного адреса 
// этого интерфейса адресу UDP - сокета
void find_broadcast_addr(char **argv, struct sockaddr_in *addr);

//послать UDP - сообщение: "Жду сообщений!"
void waiting_msgs(int sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *cli_addr, AMessage *msg);

//послать UDP - сообщение: "Есть сообщения в очереди!"
void msgs_in_queue(int sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *cli_addr, AMessage *msg);

//----------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
	int msgqid;     
	struct msqid_ds info;
	key_t key;
	key = ftok(".", 'm');
	
	int pid[PROCESS_NUM];

	printf("SERVER LW13\n");

	int TCPportno, UDPportno1, UDPportno2; // номера портов

	// ошибка в случае если мы не указали порт
    if (argc < 6) {
        fprintf(stderr,"Usage: %s <Network interface name (for example: eth0)> <Host name> <TCP port number> <UDP port number 1> <UDP port number 2>\n", argv[0]);
        exit(1);
    }

	if((TCPportno = atoi(argv[3])) == 0){
		perror("Get TCP port");
		exit(EXIT_FAILURE);
	}
	if((UDPportno1 = atoi(argv[4])) == 0){
		perror("Get UDP port 1");
		exit(EXIT_FAILURE);
	}
	if((UDPportno2 = atoi(argv[5])) == 0){
		perror("Get UDP port 2");
		exit(EXIT_FAILURE);
	}
	
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
			udp_server(argv, UDPportno1, UDPportno2,  msgqid, &info);
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
	return 0;
}

//запуск TCP - сервера
void tcp_server(char **argv, int portno, int msgqid, struct msqid_ds *info){
	int sockfd, newsockfd; // дескрипторы сокетов
	socklen_t clilen; // размер адреса клиента типа socklen_t
	struct sockaddr_in serv_addr, cli_addr;	//структуры с адресами сервера и клиента
	int pid; // id номер процесса	
	
	struct mymsgbuf qbuf;	// структура для отправки данных в очередь сообщений		

	int whoConnected = 3;

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

        if (pid < 0){
			error("ERROR on fork");
			 exit(EXIT_FAILURE);
		}            
        if (pid == 0) {			
            close(sockfd);
			int isExit = 0;
			while (!isExit){	
				
				whoConnected = who_connected(newsockfd);

				//если подключился клиент 1 типа, то получим от него данные и запишем в очередь сообщений сервера
				if(whoConnected == 1){
					
					msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений
					if((unsigned long)info->msg_qnum < (unsigned long)N){

						printf("Подключен клиент 1 типа.\n");

						AMessage *unpack = NULL;

						unpack = recieve_data_from_client1(newsockfd); // AMessage for data deserialization

						printf("Recieved data from TCP socket.\n");
						send_message_to_queue(msgqid, &qbuf, 1, unpack);
					}															

				}else if(whoConnected == 2){

					AMessage msg = AMESSAGE__INIT;  // AMessage for data serialization

					//отправим сообщение клиенту 2 типа
					printf("Подключен клиент 2 типа.\n");
					msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений

					if((unsigned long)info->msg_qnum == 0) 
					isExit = 1; //нечего отправлять, выходим					
					else {
						send_data_to_client2(newsockfd, msgqid, &msg);	
						printf("Sended data to TCP socket.\n");										
					}
				}
				else if(whoConnected == 3){					
					printf("-disconnect\n");
					nclients--;	
					exit(EXIT_SUCCESS);
				}
			}				
		}
		close(newsockfd);		
    } /* end of while */    	
	close(sockfd);
	msgctl(msgqid, IPC_RMID, 0);   
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
AMessage *recieve_data_from_client1(int sock) {	
	int bytes_recv = 0; // размер принятого сообщения		
	int dataSizeRecv = 0; // размер получаемых данных

	char buff[MAX_SEND_SIZE] = {0}; // буфер для принятых данных

	AMessage *msg;
    
   	// обработка первого параметра (получение размера данных)
	bytes_recv = recieve_all_data(sock, &dataSizeRecv, sizeof(int));
	if (bytes_recv <= 0) error("ERROR1 reading from TCP socket");
	
	//получение буфера данных
	bytes_recv = recieve_all_data(sock, buff, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR2 reading from TCP socket");		

	msg = amessage__unpack(NULL, dataSizeRecv, (uint8_t*)buff);	

	printf("Получены данные:\nразмер буфера %d байт. Строка:\n%s\n",  bytes_recv, msg->mtext);

	return msg;
}

//отправить данные клиенту 2
void send_data_to_client2(int sock, int msgqid, AMessage *msg){

	int bytes_send = 0; 		// размер отправленного сообщения				
	struct mymsgbuf queueBuf;	// буфер для полученных данных из очереди сообщений

	int len;       				// Length of serialized data	
	void *buf; 					// Buffer to store serialized data	
	
	read_message_from_queue(msgqid, &queueBuf, 0); //получим сообщение из очереди	

	//копируем данные
	msg->has_t = 1;
	msg->t = queueBuf.T;
	msg->has_len = 1;
	msg->len = queueBuf.len;
	
	//копируем строку
	char *str = calloc(msg->len, sizeof(char));
	if(!str) {
		printf("Memory allocation error!");
		exit (1);
	}
	strcpy(str, queueBuf.mtext);

	msg->mtext = str;

	printf("String for copy:\n%s\n", msg->mtext);

	len = amessage__get_packed_size(msg);
  
	buf = malloc(len);
	if(!buf) {
		printf("Memory allocation error!");
		exit (1);
	}
	amessage__pack(msg, buf);	//serialize data to buf	
	
	bytes_send = send_all_data(sock, &len, sizeof(int)); //отправка размера буфера данных клиенту
	if (bytes_send < 0) error("ERROR1 send data to socket");	
	
	bytes_send = send_all_data(sock, buf, len); //отправка буфера данных
	if (bytes_send < 0) error("ERROR2 send data to socket"); 

	printf("TCP server 2. Размер буфера в protobuf-c message: %d байт\n", len);	

	if(buf != NULL) free(buf);
	if(str != NULL) free(str);	
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
void send_message_to_queue(int qid, struct mymsgbuf *qbuf, long type, AMessage *unpack){
	int length = 0;	

	qbuf->mtype = type;
	qbuf->T = unpack->t;
	qbuf->len = unpack->len;
	
	strcpy(qbuf->mtext, unpack->mtext);
	
	length = sizeof(struct mymsgbuf) - sizeof(long);

	//отправим сообщение
	if((msgsnd(qid, qbuf, length, 0)) ==-1){
			perror("msgsnd");
			exit(1);
	}
	printf("Data sended to queue...\n");

	// Free the unpacked message
  	amessage__free_unpacked(unpack, NULL);
}

//принять данные из списка сообщений IPC сервера
void read_message_from_queue(int qid, struct mymsgbuf *qbuf, long type){
	
	int msgSize = sizeof(struct mymsgbuf) - sizeof(long);	
	qbuf->mtype = type;
	msgrcv(qid, qbuf, msgSize, type, 0);       
	printf("MESSAGE RECEIVED from queue...\n");
}

//запуск UDP - сервера
void udp_server(char **argv, int portno1, int portno2, int msgqid, struct msqid_ds *info){
	int sockfd1, sockfd2; // дескриптор сокета	
	struct sockaddr_in local_addr;	//структура с локальным адресом 
	int pid[PROCESS_NUM]; // id номера процессов

	AMessage msg = AMESSAGE__INIT;  // AMessage	
	
	//так как клиенты создаются на одном компьютере создадим 2 сокета	
	for (int i = 0; i < PROCESS_NUM; i++) { 
		pid[i] = fork();
        if (pid[i] < 0)
            error("ERROR on fork");
        if (pid[i] == 0) {

			if(!i){
				create_socket_UDP(&sockfd1);
				bind_UDP_socket_to_local_addr(argv, &sockfd1, &local_addr, portno1); 
				printf("UDP socket %d created.\n", ++i);
				while(1){					
					waiting_msgs(sockfd1, msgqid, info, &local_addr, &msg);
				}	
				close(sockfd1);					

			}else {
				create_socket_UDP(&sockfd2);				
				bind_UDP_socket_to_local_addr(argv, &sockfd2, &local_addr, portno2);
				printf("UDP socket %d created.\n", ++i);
				while(1) {					
					msgs_in_queue(sockfd2, msgqid, info, &local_addr, &msg);
				}
				close(sockfd2);
			}
		}
	}
	for (int i = 0; i < PROCESS_NUM; i++) {
		int stat;
        int status = waitpid(pid[i], &stat, 0);
        if (pid[i] == status) {
			if(!i)
            printf("UDP cервер 1 отключен, result=%d\n", WEXITSTATUS(stat));
			else
			printf("UDP cервер 2 отключен, result=%d\n", WEXITSTATUS(stat));
            fflush(stdout);
        }
    }	
}

//послать UDP - сообщение: "Жду сообщений!"
void waiting_msgs(int sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *local_addr, AMessage *msg){
	
	unsigned len;               // Length of serialized data	
	void *buf; 					// Buffer to store serialized data	

	int n = 0;
	int isExit = 0;	

	msg->mtext = msgToRecieve;	

	len = amessage__get_packed_size(msg);
  
	buf = malloc(len);
	amessage__pack(msg, buf);	//serialize data to buf	

	socklen_t size = sizeof(struct sockaddr_in);	

	while (!isExit){
		sleep(K);
		msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений
		if((unsigned long)info->msg_qnum >= (unsigned long)N) isExit = 1;
		else{			
			n = sendto(sockfd, buf, len, 0, (struct sockaddr *)local_addr, size);
			if (n < 0) error("sendto");
			printf("Sended \"Waiting data\" message to UDP socket.\n");	
		}			
	}
	if(buf != NULL) free(buf);

	printf("Too many messages in queue...\n");				
}

//послать UDP - сообщение: "Есть сообщения в очереди!"
void msgs_in_queue(int sockfd, int msgqid, struct msqid_ds *info, struct sockaddr_in *cli_addr, AMessage *msg){

	unsigned len;               // Length of serialized data	
	void *buf; 					// Buffer to store serialized data		

	msg->mtext = msgToSend;	

	len = amessage__get_packed_size(msg);
  
	buf = malloc(len);
	amessage__pack(msg, buf);	//serialize data to buf	

	int n = 0;
	int isExit = 0;		
	socklen_t size = sizeof(struct sockaddr_in);
			
	while (!isExit){
		sleep(L);
		msgctl(msgqid, IPC_STAT, info); //получить содержимое структуры msqid_ds для текущей очереди сообщений
		if((unsigned long)info->msg_qnum == 0) isExit = 1;
		else{			
			n = sendto(sockfd, buf, len, 0,(struct sockaddr *)cli_addr, size);
			if (n < 0) error("sendto");
			printf("Sended \"Have data for sending\" message to UDP socket.\n");
		}			
	}
	if(buf != NULL) free(buf);

	printf("No messages in queue...\n");
}

//создать сокет с отправкой данных по UDP - протоколу
void create_socket_UDP(int *sockfd){
	*sockfd = socket(AF_INET, SOCK_DGRAM, 0);	
    // ошибка при создании сокета
	if (*sockfd < 0) 
       error("ERROR opening socket");
}

//связать сокет с определенным адресом
void bind_UDP_socket_to_local_addr(char **argv, int *UDPsock, struct sockaddr_in *addr, int portno){
		
	int broadcastEnable=1;
	int ret=setsockopt(*UDPsock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	if(ret<0){
		perror("setsockopt broadcast enable");
		exit(EXIT_FAILURE);
	}

	// заполнение структуры serv_addr
    memset(addr, 0, sizeof(struct sockaddr_in));	
    addr->sin_family = AF_INET;
	find_broadcast_addr(argv, addr); // присвоить широковещательный адрес
	addr->sin_port = htons(portno);	
}

// поиск в сетевых интерфейсах локального компьютера интерфейса,
// указанного в параметрах командной строки с присвоением широковещательного адреса 
// этого интерфейса адресу UDP - сокета
void find_broadcast_addr(char **argv, struct sockaddr_in *addr){
	const char* ifname = "enp3s0";  
    struct ifaddrs *ifaddr, *ifa;
	ifaddr = NULL;
	int addrIsFinded = 0;

	ifname = argv[1];  

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

        if((ifa->ifa_addr != NULL) &&
           (strcmp(ifa->ifa_name, ifname) == 0) &&  
           (ifa->ifa_addr->sa_family == AF_INET)) {		

            printf("\tInterface: <%s>\n",ifa->ifa_name ); 
            printf("\tBroadcast address: <%s>\n", inet_ntoa(((struct sockaddr_in *)ifa->ifa_broadaddr)->sin_addr));

			addr->sin_addr.s_addr = ((struct sockaddr_in *)ifa->ifa_broadaddr)->sin_addr.s_addr;
			addrIsFinded = 1;
            break;
        }
    }
	freeifaddrs(ifaddr);
	if(!addrIsFinded){
		printf("Broadcast address not finded!\n");
		exit(EXIT_FAILURE);
	}
}

//связать сокет с определенным адресом
void bind_TCP_socket_to_local_addr(char **argv, int *sockfd, struct sockaddr_in *addr, int portno){
	struct hostent *hp; // структура хоста
	memset(addr, 0, sizeof(struct sockaddr_in));	
    addr->sin_family = AF_INET;
	hp = gethostbyname(argv[2]); // извлечение хоста
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