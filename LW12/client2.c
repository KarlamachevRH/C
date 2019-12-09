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

//вывод сообщения об ошибке
void error(const char *msg); 

//создание UDP сокета
void create_socket_UDP(int *UDPsockfd);

//инициализация данных для созданного UDP сокета
void get_data_for_UDP_socket(char **argv, int *UDPsock, int UDPportno, struct sockaddr_in *serv_addr_UDP);

//инициализация данных адреса сервера
void set_serv_addr_data_UDP( struct sockaddr_in *serv_addr);
void set_serv_addr_data_TCP(char **argv, struct sockaddr_in *serv_addr, struct hostent *server);

//создать TCP сокет
void create_socket_TCP(int *TCPsockfd);

//инициализация адресных данных TCP сокета и соединение с сервером
void get_data_for_TCP_socket(char **argv, int *TCPsock, int TCPportno, struct sockaddr_in *serv_addr_TCP, struct hostent *server);

//получить блок данных от сервера
void get_data_from_TCP_socket(int sock, struct TCPmsgbuf *buff);

//прочитаем файл с текстом и возьмем оттуда случайную строку
void save_string_to_file(char *str);

//отправить данные на TCP - сокет сервера
int send_data_to_TCP_serv(int sock, void *buff, int len);

// получить буфер известного размера с UDP сокета
int recieve_data_from_UDP_serv(int sock, void *buff, int len, struct sockaddr_in *serv_addr_UDP);

// получить буфер известного размера с TCP - сокета сервера
int recieve_data_from_TCP_serv(int sock, void *buff, int len);



int main(int argc, char *argv[]) {
	int UDPsock, TCPsock; 								// дескрипторы сокетов	
	struct sockaddr_in serv_addr_TCP, serv_addr_UDP;	// структура с адресом сервера
    struct hostent server; 							// структура хоста
	int UDPportno, TCPportno; 							// номер порта
	
	char buffer[BUFF_SIZE]; //buffer for text message from udp server
	char quitBuff[BUFF_SIZE] = {0};
	struct TCPmsgbuf dataBlock; //блок данных для отправки серверу

	int clientNum = 2;


	FILE *fd = fopen("recieved_text", "w");
	if (fd == NULL) {		
		error("Ошибка окрытия файла");
		exit(-1);
	}
	fclose(fd);

	printf("CLIENT2 LW12\n");

	// ошибка в случае если мы не указали порт
    if (argc < 4) {
        fprintf(stderr,"Usage: %s <Server name> <TCP port number> <UDP port number>\n", argv[0]);
        exit(1);
    }

	// извлечение портов
	TCPportno = atoi(argv[2]);
	UDPportno = atoi(argv[3]);
	// создание сокетов клиента 
	get_data_for_UDP_socket(argv, &UDPsock, UDPportno, &serv_addr_UDP);
	get_data_for_TCP_socket(argv, &TCPsock, TCPportno, &serv_addr_TCP, &server);

    while(1){		
		
		if((recieve_data_from_UDP_serv(UDPsock, buffer, BUFF_SIZE, &serv_addr_UDP))>0){
			
			//если получено сообщение о наличии данных, то получим данные от TCP - сокета сервера
			if(strcmp(buffer, msgToSend) == 0){ 				

				send_data_to_TCP_serv(TCPsock, &clientNum, sizeof(int));

				get_data_from_TCP_socket(TCPsock, &dataBlock);

				save_string_to_file(dataBlock.mtext);

				printf("Клиент 2. Обработка данных T = %d сек.\n", dataBlock.t);

				sleep(dataBlock.t);				
			}			
		} 	

		printf("Enter \"quit\" for exit or other any symbols for continue:\n");

		fgets(&quitBuff[0], sizeof(quitBuff) - 1, stdin);
		// проверка на "quit"
		if (!strcmp(&quitBuff[0], "quit\n")) {
			// Корректный выход
			printf("Exit from client 2...\n");
			clientNum = 3;
			send_data_to_TCP_serv(TCPsock, &clientNum, sizeof(int));
			close(UDPsock);
			close(TCPsock);
			exit(0);
		}

	}  
	exit(EXIT_SUCCESS);
}

//инициализация данных для созданного UDP сокета
void get_data_for_UDP_socket(char **argv, int *UDPsock, int UDPportno, struct sockaddr_in *serv_addr_UDP){

	create_socket_UDP(UDPsock);

    set_serv_addr_data_UDP(serv_addr_UDP);
    
    // установка порта
	serv_addr_UDP->sin_port = htons(UDPportno);

	// вызываем bind для связывания
    if (bind(*UDPsock, (struct sockaddr *)serv_addr_UDP, sizeof(struct sockaddr_in)))
    {error("binding");}
}

//создание UDP сокета
void create_socket_UDP(int *UDPsockfd){
	*UDPsockfd = socket(AF_INET, SOCK_DGRAM, 0);	
    // ошибка при создании сокета
	if (*UDPsockfd < 0) 
       error("ERROR opening socket");
}

//инициализация данных адреса сервера
void set_serv_addr_data_UDP(struct sockaddr_in *serv_addr){
	
    // заполнение структуры serv_addr
    memset(serv_addr, 0, sizeof(struct sockaddr_in));	
    serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;    
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
void get_data_from_TCP_socket(int sock, struct TCPmsgbuf *buff){
	int bytes_recv = 0; // размер принятого сообщения		
	int dataSizeRecv = 0; // размер получаемых данных		
    
   	// обработка первого параметра (получение размера структуры)
	bytes_recv = recieve_data_from_TCP_serv(sock, &dataSizeRecv, sizeof(int));
	if (bytes_recv <= 0) error("ERROR1 reading from socket");
	
	//получение структуры с данными (без строки)
	bytes_recv = recieve_data_from_TCP_serv(sock, buff, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR2 reading from socket");	

	//получение размера строки
	bytes_recv = recieve_data_from_TCP_serv(sock, &dataSizeRecv, sizeof(int));	
	if (bytes_recv <= 0) error("ERROR3 reading from socket");

	printf("String size for reading from server: %d\n", dataSizeRecv);

	// выделяем память для строки после получения структуры с указателем на строку
	buff->mtext = calloc(buff->len, sizeof(char));	 
	if(!buff->mtext) {
		printf("Memory allocation error!");
		exit (1);
	}
	
	//получение строки
	bytes_recv = recieve_data_from_TCP_serv(sock, buff->mtext, dataSizeRecv);
	if (bytes_recv < 0) error("ERROR4 reading from socket");	

	printf("Recieved text string:\n%s", buff->mtext);
}

//прочитаем файл с текстом и возьмем оттуда случайную строку
void save_string_to_file(char *str){	

	FILE *fd = fopen("recieved_text", "a");
	if (fd == NULL) {		
		error("Ошибка окрытия файла");
		exit(-1);
	}	
	
	if (fputs(str, fd)==-1)
        error ("Ошибка вывода");

	if (fputc('\n', fd)==-1)
        error ("Ошибка вывода");	

	fclose(fd);
	if(str != NULL) free(str);

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
int recieve_data_from_UDP_serv(int sock, void *buff, int len, struct sockaddr_in *serv_addr_UDP){
	
    int n = 1;
	socklen_t size = sizeof(struct sockaddr_in);	
    	
	n = recvfrom(sock, buff, len, 0, (struct sockaddr *)&serv_addr_UDP, &size);
	if (n < 0) error("recvfrom");    
   
    return n;
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