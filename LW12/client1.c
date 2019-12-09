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

#define T 6

//вывод сообщения об ошибке
void error(const char *msg); 

//создание UDP сокета
void create_socket_UDP(int *UDPsockfd);

//инициализация данных для созданного UDP сокета
void get_data_for_UDP_socket(int *UDPsock, int UDPportno, struct sockaddr_in *serv_addr_UDP);

//инициализация данных адреса сервера
void set_serv_addr_data_UDP( struct sockaddr_in *serv_addr);
void set_serv_addr_data_TCP(char **argv, struct sockaddr_in *serv_addr, struct hostent *server);

//создать TCP сокет
void create_socket_TCP(int *TCPsockfd);

//инициализация адресных данных TCP сокета и соединение с сервером
void get_data_for_TCP_socket(char **argv, int *TCPsock, int TCPportno, struct sockaddr_in *serv_addr_TCP, struct hostent *server);

//сгенерировать блок данных для отправки на сервер
void generate_data_block(struct TCPmsgbuf *buf);

//прочитаем файл с текстом и возьмем оттуда случайную строку
void read_file_and_get_string(char *rawString, int length);

//посчитаем количество строк в файле данных
int count_strings_in_file(FILE *fd, char *rawString, int length);

//получим случайную строку из файла
void get_string(FILE *fd, char *rawString, int length, int cnt);

// отправить весь буфер целиком на TCP сокет
int send_data_to_TCP_serv(int sock, void *buff, int len);

// получить буфер известного размера с UDP сокета
int recieve_data_from_UDP_serv(int *sock, void *buff, int len);



int main(int argc, char *argv[]) {
	int UDPsock, TCPsock; 								// дескрипторы сокетов	
	struct sockaddr_in serv_addr_TCP, serv_addr_UDP;	// структура с адресом сервера
    struct hostent server; 							// структура хоста
	int UDPportno, TCPportno; 							// номер порта
	
	char buffer[BUFF_SIZE+1]; //buffer for text message from udp server
	struct TCPmsgbuf dataBlock; //блок данных для отправки серверу
	int clientNum = 1;

	char quitBuff[BUFF_SIZE] = {0};

	printf("CLIENT1 LW12\n");

	// ошибка в случае если мы не указали порт
    if (argc < 4) {
        fprintf(stderr,"Usage: %s <Server name> <TCP port number> <UDP port number>\n", argv[0]);
        exit(1);
    }

	// извлечение портов
	TCPportno = atoi(argv[2]);
	UDPportno = atoi(argv[3]);

	// создание сокетов клиента 
	get_data_for_UDP_socket(&UDPsock, UDPportno, &serv_addr_UDP);	
	get_data_for_TCP_socket(argv, &TCPsock, TCPportno, &serv_addr_TCP, &server); //с подключением к TCP - сокету сервера

	int n = 0;

    while(1){		
		
		n = recieve_data_from_UDP_serv(&UDPsock, buffer, BUFF_SIZE);

		if(n > 0){
			printf("Клиент 1. Принято сообщение: %s\n", buffer);
			if((strcmp(buffer, msgToRecieve)) == 0){

				generate_data_block(&dataBlock); //создать блок данных для отправки на TCP - сокет

				printf("Клиент 1. Создано сообщение:\n%s\n", dataBlock.mtext);

				send_data_to_TCP_serv(TCPsock, &clientNum, sizeof(int)); //отправим номер клиента
				
				int len = (int)sizeof(dataBlock);								

				//отправка размера структуры
				send_data_to_TCP_serv(TCPsock, &len, sizeof(int));	
				
				//отправка структуры
				send_data_to_TCP_serv(TCPsock, &dataBlock, len);

				len = dataBlock.len;

				printf("Клиент 1. Размер строки в dataBlock: %d байт\n", dataBlock.len);

				//отправка размера строки
				send_data_to_TCP_serv(TCPsock, &len, sizeof(int));	
				
				//отправка строки
				send_data_to_TCP_serv(TCPsock, dataBlock.mtext, len);

				if(dataBlock.mtext != NULL) free(dataBlock.mtext );

				printf("Клиент 1. Обработка данных T = %d сек.\n", dataBlock.t);
				sleep(dataBlock.t);
			}
		} 
		printf("Enter \"quit\" for exit or other any symbols for continue:\n");

		fgets(&quitBuff[0], sizeof(quitBuff) - 1, stdin);
		// проверка на "quit"
		if (!strcmp(&quitBuff[0], "quit\n")) {
			// Корректный выход
			printf("Exit from client 1...\n");
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
void get_data_for_UDP_socket(int *UDPsock, int UDPportno, struct sockaddr_in *serv_addr_UDP){

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

//создать TCP сокет
void create_socket_TCP(int *TCPsockfd){
	*TCPsockfd = socket(AF_INET, SOCK_STREAM, 0);	
    // ошибка при создании сокета
	if (*TCPsockfd < 0) 
       error("ERROR opening socket");
}

//инициализация адресных данных TCP сокета и соединение с сервером
void get_data_for_TCP_socket(char **argv, int *TCPsock, int TCPportno, struct sockaddr_in *serv_addr_TCP, struct hostent *server){
	
	// создание сокета 
	create_socket_TCP(TCPsock);
	set_serv_addr_data_TCP(argv, serv_addr_TCP, server);
	// установка порта
	serv_addr_TCP->sin_port = htons(TCPportno);
	 
	/// установка соединения	
	if (connect(*TCPsock,(struct sockaddr *)serv_addr_TCP, sizeof(struct sockaddr_in)) < 0) 
        error("ERROR connecting");
}

void error(const char *msg) {
    perror(msg);
    exit(0);
}

//сгенерировать блок данных для отправки на сервер
void generate_data_block(struct TCPmsgbuf *buf){
	srand(time(NULL));
	buf->t = rand() % T;	

	char rawString[MAX_SEND_SIZE];
	memset(rawString, 0, MAX_SEND_SIZE);

	read_file_and_get_string(rawString, MAX_SEND_SIZE);	
	
	if((strlen(rawString)+1) > MAX_SEND_SIZE){
		buf->len = rand() % MAX_SEND_SIZE;
		rawString[(buf->len)-1] = '\0';
	}else{		
		buf->len = strlen(rawString);	
		rawString[(buf->len)-1] = '\0';	//убрать '\n' из строки
	}

	buf->mtext = calloc(buf->len, sizeof(char));
	strcpy(buf->mtext, rawString);	
}

//прочитаем файл с текстом и возьмем оттуда случайную строку
void read_file_and_get_string(char *rawString, int length){
	FILE *fd;	
	int cnt = 0;

	fd = fopen("fish", "r");
	if (fd == NULL) {		
		printf ("Ошибка окрытия файла!\n"); 
		exit(EXIT_FAILURE);
	}

	cnt = count_strings_in_file(fd, rawString, MAX_SEND_SIZE);

	fseek(fd, 0, SEEK_SET); //переместить указатель внутри файла на начало
	
	get_string(fd, rawString, length, cnt);

	fclose(fd);	
}

//посчитаем количество строк в файле данных
int count_strings_in_file(FILE *fd, char *rawString, int length){
	int cnt = 0;	
	char *errCheck;
	while (!feof(fd)){
		// Чтение одной строки  из файла
		errCheck = fgets (rawString, length, fd);
		if (errCheck != NULL) 
			cnt++;
	}
	return cnt;
}

//получим случайную строку из файла
void get_string(FILE *fd, char *rawString, int length, int cnt){
	int strNum = 0;	
	char *errCheck;
	srand(time(NULL));
	strNum = rand() % cnt; //получим номер случайной строки из файла	
	cnt = 0; //обнулим счетчик строк
	int isEndSearch = 0;

	while (!isEndSearch && !feof(fd)){
		// Чтение одной строки  из файла
		errCheck = fgets (rawString, length, fd);
		if (errCheck != NULL){
			cnt++;
			if(strNum == cnt) //если случайная строка найдена, то выходим
				isEndSearch = 1;
		}else {
			if (feof (fd) != 0)  				
				printf ("Чтение файла закончено\n");				
			else 
				error("Ошибка чтения из файла");
		}		
    }
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

// получить буфер известного размера
int recieve_data_from_UDP_serv(int *sock, void *buff, int len){

    int n = 0;	
	struct sockaddr_in server;
	socklen_t size = sizeof(server);
   	
	n = recvfrom(*sock, buff, len, 0,(struct sockaddr *)&server, &size);
	if (n < 0) error("recvfrom");
	  	
	//printf("Клиент 1. Приняты данные %d байт\n", n);	
    return n;
}