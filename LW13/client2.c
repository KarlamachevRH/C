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

#define fname "recieved_text"

//вывод сообщения об ошибке
void error(const char *msg); 

//создание UDP сокета
void create_socket_UDP(int *UDPsockfd);

//инициализация данных адреса сервера
void set_serv_addr_data_UDP(char **argv, int UDPsock, struct sockaddr_in *serv_addr, int UDPportno);

//инициализация данных адреса сервера
void set_serv_addr_data_TCP(char **argv, struct sockaddr_in *serv_addr, struct hostent *server);

//создать TCP сокет
void create_socket_TCP(int *TCPsockfd);

//инициализация адресных данных TCP сокета и соединение с сервером
void get_data_for_TCP_socket(char **argv, int *TCPsock, int TCPportno, struct sockaddr_in *serv_addr_TCP, struct hostent *server);

//получить блок данных от сервера
AMessage *get_data_from_TCP_socket(int sock);

//сохраним в файл полученный от сервера текст
void save_string_to_file(AMessage *msg);

//отправить данные на TCP - сокет сервера
int send_data_to_TCP_serv(int sock, void *buff, int len);

// получить буфер известного размера с UDP сокета
int recieve_data_from_UDP_serv(int sock, char *str);

// получить буфер известного размера с TCP - сокета сервера
int recieve_data_from_TCP_serv(int sock, void *buff, int len);



int main(int argc, char *argv[]) {
	int UDPsock, TCPsock; 								// дескрипторы сокетов	
	struct sockaddr_in serv_addr_TCP, serv_addr_UDP;	// структура с адресом сервера
    struct hostent server; 								// структура хоста
	int UDPportno, TCPportno; 							// номер порта
	
	char UDPmsg[BUFF_SIZE] = {0}; // буфер для сообщения от UDP - сервера		

	int clientNum = 2; 	// Номер клиента для идентификации на TCP - сервере

	AMessage *msg = NULL;  // AMessage for data unpack

	FILE *fd = fopen(fname, "w"); // создадим новый файл для принятых через TCP - сокет данных
	if (fd == NULL) {		
		error("Ошибка окрытия файла");
		exit(-1);
	}
	fclose(fd);

	printf("CLIENT2 LW13\n");

	// ошибка в случае если мы не указали порт
    if (argc < 4) {
        fprintf(stderr,"Usage: %s <TCP host name> <TCP port number> <UDP port number>\n", argv[0]);
        exit(1);
    }

	// извлечение портов
	if((TCPportno = atoi(argv[2])) == 0){
		perror("Get TCP port");
		exit(EXIT_FAILURE);
	}
	if((UDPportno = atoi(argv[3])) == 0){
		perror("Get UDP port");
		exit(EXIT_FAILURE);
	}

	
	// создание UDP сокета и инициализация данных адреса
	create_socket_UDP(&UDPsock);

	set_serv_addr_data_UDP(argv, UDPsock, &serv_addr_UDP, UDPportno);

    while(1){		
		
		if((recieve_data_from_UDP_serv(UDPsock, UDPmsg))>0){
			
			//если получено сообщение о наличии данных, то получим данные от TCP - сокета сервера
			if(strcmp(UDPmsg, msgToSend) == 0){ 

				get_data_for_TCP_socket(argv, &TCPsock, TCPportno, &serv_addr_TCP, &server);

				send_data_to_TCP_serv(TCPsock, &clientNum, sizeof(int));

				msg = get_data_from_TCP_socket(TCPsock);

				int t = msg->t; // время "обработки" данных клиентом

				save_string_to_file(msg);

				printf("Клиент 2. Обработка данных T = %d сек.\n", t);

				sleep(t);

				close(TCPsock);				
			}			
		} 		
	}  
	printf("Exit from client 2...\n");		
	clientNum = 3;	
	send_data_to_TCP_serv(TCPsock, &clientNum, sizeof(int));
	close(UDPsock);	
	exit(EXIT_SUCCESS);
}

//создание UDP сокета
void create_socket_UDP(int *UDPsockfd){
	*UDPsockfd = socket(AF_INET, SOCK_DGRAM, 0);	
    // ошибка при создании сокета
	if (*UDPsockfd < 0) 
       error("ERROR opening socket");
}

//инициализация данных адреса сервера
void set_serv_addr_data_UDP(char **argv, int UDPsock, struct sockaddr_in *serv_addr, int UDPportno){
	
	memset(serv_addr, 0, sizeof(struct sockaddr_in));	
    serv_addr->sin_family = AF_INET;

	serv_addr->sin_addr.s_addr = INADDR_ANY;	

    serv_addr->sin_port = htons(UDPportno); 	

	// вызываем bind для связывания
    if (bind(UDPsock, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)))
    {error("binding");}  
}

//инициализация адресных данных TCP сокета и соединение с сервером
void get_data_for_TCP_socket(char **argv, int *TCPsock, int TCPportno, struct sockaddr_in *serv_addr_TCP, struct hostent *server){
	
	// создание сокета 
	create_socket_TCP(TCPsock);
	set_serv_addr_data_TCP(argv, serv_addr_TCP, server);
	// установка порта
	serv_addr_TCP->sin_port = htons(TCPportno);
	 
	/// установка соединения	
	if (connect(*TCPsock,(struct sockaddr *) serv_addr_TCP, sizeof(struct sockaddr_in)) < 0) 
        error("ERROR connecting");
}

//создать TCP сокет
void create_socket_TCP(int *TCPsockfd){
	*TCPsockfd = socket(AF_INET, SOCK_STREAM, 0);	
    // ошибка при создании сокета
	if (*TCPsockfd < 0) 
       error("ERROR opening socket");
}

//получить и установить данные адреса  TCP - сервера
void set_serv_addr_data_TCP(char **argv, struct sockaddr_in *serv_addr, struct hostent *server){
	 // извлечение хоста
	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // заполнение структуры serv_addr
    memset(serv_addr, 0, sizeof(struct sockaddr_in));	
    serv_addr->sin_family = AF_INET;	
    memmove(&serv_addr->sin_addr.s_addr, server->h_addr, server->h_length);
}

void error(const char *msg) {
    perror(msg);
    exit(0);
}

//получить блок данных от сервера
AMessage *get_data_from_TCP_socket(int sock){

	int bytes_recv = 0; // размер принятого сообщения		
	int dataSizeRecv = 0; // размер получаемых данных		 

	char buff[MAX_SEND_SIZE] = {0}; // буфер для принятых данных

	AMessage *msg;

   	// обработка первого параметра (получение размера буфера)
	bytes_recv = recieve_data_from_TCP_serv(sock, &dataSizeRecv, sizeof(int));
	if (bytes_recv < 0) error("ERROR1 reading from socket");
	
	//получение буфера
	bytes_recv = recieve_data_from_TCP_serv(sock, buff, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR2 reading from socket");	

	msg = amessage__unpack(NULL, dataSizeRecv, (uint8_t*)buff);

	printf("Recieved text string:\n%s", msg->mtext);

	return msg;
}

// получить буфер известного размера с TCP - сокета сервера
int recieve_data_from_TCP_serv(int sock, void *buff, int len){
	
	int total = 0;
    int n = 0;

    while(total < len && n >= 0) {		
        n = recv(sock, buff+total, len-total, 0);  
        total += n;
    }	
    return total;
}

//прочитаем файл с текстом и возьмем оттуда случайную строку
void save_string_to_file(AMessage *msg){	

	FILE *fd = fopen(fname, "a");
	if (fd == NULL) {		
		error("Ошибка окрытия файла");
		exit(-1);
	}	
	
	if (fputs(msg->mtext, fd)==-1)
        error ("Ошибка вывода");

	if (fputc('\n', fd)==-1)
        error ("Ошибка вывода");	

	fclose(fd);
	
	// Free the unpacked message
 	amessage__free_unpacked(msg, NULL);

	printf("String saved to file\n");
}

// отправить весь буфер целиком
int send_data_to_TCP_serv(int sock, void *buff, int len) {
    int total = 0;
    int n = 0;

    while(total < len && n >= 0) {
        n = send(sock, buff+total, len-total, 0);        
        total += n;
    }
    return (n==-1 ? -1 : total);
}

// получить буфер известного размера с UDP - сокета сервера
int recieve_data_from_UDP_serv(int sock, char *str){

	AMessage *msg;

	char buff[BUFF_SIZE] = {0}; // буфер для принятых данных

    int n = 0;	
	struct sockaddr_in serv_addr;
	socklen_t size = sizeof(serv_addr);
   	
	n = recvfrom(sock, buff, BUFF_SIZE, 0,(struct sockaddr *)&serv_addr, &size);
	if (n < 0) error("recvfrom");	  	

	msg = amessage__unpack(NULL, n, (uint8_t*)buff);
	if (msg == NULL){		
		fprintf(stderr, "Error unpacking incoming UDP message\n");
		exit(1);
	}

	strcpy(str, msg->mtext);

	// Free the unpacked message
 	amessage__free_unpacked(msg, NULL);
		
    return n;
}