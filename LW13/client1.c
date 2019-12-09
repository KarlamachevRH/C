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

#define fname "fish"

#define T 6

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

//сгенерировать блок данных для отправки на сервер
void generate_data_block(AMessage *msg, char *str);

//прочитаем файл с текстом и возьмем оттуда случайную строку
void read_file_and_get_string(char *rawString, int length);

//посчитаем количество строк в файле данных
int count_strings_in_file(FILE *fd, char *rawString, int length);

//получим случайную строку из файла
void get_string(FILE *fd, char *rawString, int length, int cnt);

// Отправить сериализованные данные на TCP - сокет
void send_msg_to_TCP_serv(int TCPsock, AMessage *msg);

// отправить весь буфер целиком на TCP сокет
int send_data_to_TCP_serv(int sock, void *buff, int len);

// получить буфер известного размера с UDP сокета
int recieve_data_from_UDP_serv(int sock, char *UDPmsg);


int main(int argc, char *argv[]) {
	int UDPsock, TCPsock; 								// дескрипторы сокетов	
	struct sockaddr_in serv_addr_TCP, serv_addr_UDP;	// структура с адресом сервера
    struct hostent server; 								// структура хоста
	int UDPportno, TCPportno; 							// номер порта
	
	char UDPmsg[BUFF_SIZE] = {0};	// буфер для сообщения от UDP - сервера	

	printf("CLIENT1 LW13\n");

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

	//connect(UDPsock, (struct sockaddr *)&serv_addr_UDP, sizeof(serv_addr_UDP));

	int n = 0;
	char *str = NULL;

    while(1){		
		
		n = recieve_data_from_UDP_serv(UDPsock, UDPmsg);

		if(n > 0){
			printf("Клиент 1. Принято сообщение: %s\n", UDPmsg);
			if((strcmp(UDPmsg, msgToRecieve)) == 0){

				get_data_for_TCP_socket(argv, &TCPsock, TCPportno, &serv_addr_TCP, &server); // создание сокета TCP с подключением к TCP - сокету сервера
				
				AMessage msg = AMESSAGE__INIT;  // AMessage					
				
				generate_data_block(&msg, str); //создать блок данных для отправки на TCP - сокет

				int t = msg.t; // время "обработки" данных клиентом

				printf("Клиент 1. Создано сообщение:\n%s\n", msg.mtext);

				send_msg_to_TCP_serv(TCPsock, &msg); //отправка сериализованных данных на  TCP - сокет

				if(str != NULL) free(str);	

				printf("Клиент 1. Обработка данных T = %d сек.\n", t);
				sleep(t);
				close(TCPsock);	 
			}
		} 
		if(str != NULL) free(str);			
	}	
	printf("Exit from client 1...\n");			
	int clientIsOff = 3;
	send_data_to_TCP_serv(TCPsock, &clientIsOff, sizeof(int));
	close(UDPsock);	
	exit(EXIT_SUCCESS);
}

//создание UDP сокета
void create_socket_UDP(int *UDPsockfd){
	*UDPsockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	
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
void generate_data_block(AMessage *msg, char *str){
	srand(time(NULL));
	msg->has_t = 1;
	msg->t = rand() % T;		

	char rawString[MAX_SEND_SIZE];
	memset(rawString, 0, MAX_SEND_SIZE);

	read_file_and_get_string(rawString, MAX_SEND_SIZE);	
	
	msg->has_len = 1;

	if((strlen(rawString)+1) > MAX_SEND_SIZE){
		msg->len = rand() % MAX_SEND_SIZE;
		rawString[(msg->len)-1] = '\0';
	}else{		
		msg->len = strlen(rawString);	 //string length + '\n'
		rawString[(msg->len)-1] = '\0';	 //убрать '\n' из строки		
	}

	str = calloc(strlen(rawString)+1, sizeof(char));
	if(!str) {
		printf("Memory allocation error!");
		exit (1);
	}
	strcpy(str, rawString);

	msg->mtext = str;	
}

//прочитаем файл с текстом и возьмем оттуда случайную строку
void read_file_and_get_string(char *rawString, int length){
	FILE *fd;	
	int cnt = 0;

	fd = fopen(fname, "r");
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

// Отправить сериализованные данные на TCP - сокет
void send_msg_to_TCP_serv(int TCPsock, AMessage *msg){

	int len = 0;                // Length of serialized data
	int clientNum = 1;			// Номер клиента для идентификации на TCP - сервере	
	void *buf; 					// Buffer to store serialized data	

	send_data_to_TCP_serv(TCPsock, &clientNum, sizeof(int)); //отправим номер клиента	

	len = amessage__get_packed_size(msg);
  
	buf = malloc(len);
	if(!buf) {
		printf("Memory allocation error!");
		exit (1);
	}

	amessage__pack(msg, buf);	//serialize data to buf								

	//отправка размера буфера данных
	send_data_to_TCP_serv(TCPsock, &len, sizeof(int));	
	
	//отправка буфера данных
	send_data_to_TCP_serv(TCPsock, buf, len);				

	printf("Клиент 1. Размер буфера в protobuf-c message: %d байт\n", len);				

	if(buf != NULL) free(buf);		
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
int recieve_data_from_UDP_serv(int sock, char *str){	

	AMessage *msg;

	char buff[BUFF_SIZE] = {0}; // буфер для принятых данных

	int n = 0;	
      	
	/* n = recv(sock, buff, BUFF_SIZE, 0);  
	if (n < 0) error("recv");     */
	
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