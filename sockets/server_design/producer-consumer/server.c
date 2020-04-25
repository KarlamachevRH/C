#include "headers.h"

#define SERVERS_NUM 40

#define MAXNCLI 30 /* Максимальная очередь клиентов */
int clifd[MAXNCLI], iget, iput; /* Массив дескрипторов сокетов для клиентов в очереди и индексы дескрипторов для помещения в очередь (iput), изьятия из очереди (iget) */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


/* Печать адреса клиента */
static void print_client_data(struct sockaddr_in cli_addr)
{
	char client_addr[CLIENT_ADDR_LEN];
	printf("Client address: %s:%d\n", \
			inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, client_addr, CLIENT_ADDR_LEN), \
			ntohs(cli_addr.sin_port));
}

/* Получим строку с текущей датой */
static char *get_date()
{
	char s[DATE_STR_LEN] = {0};
	char *date = NULL;
	struct tm *t = NULL;
	time_t timer;

	timer = time(NULL);
	if(timer == ((time_t)-1))
		handle_error("timer");

	t = localtime(&timer);
	if(t == NULL)
		handle_error("localtime");

	if((strftime(s, DATE_STR_LEN, "%d.%m.%Y %H:%M:%S, %A", t)) < 0)
	{
		printf("Can't get date\n");
		exit(EXIT_FAILURE);
	}
	date = calloc(DATE_STR_LEN, sizeof(char));
	strncpy(date, s, DATE_STR_LEN);
	return date;
}

static void *request_handler(void *arg)
{
	int cfd; /* Сonnected socket */
	int bytes_sended = 0;
	int bytes_recieved = 0;
	char str_for_send[DATE_STR_LEN] = {0};
	char str_for_receive[DATE_STR_LEN] = {0};
	int err;

	char *date = NULL;
	date = get_date();
	snprintf(str_for_send, DATE_STR_LEN, "%s", date);
	if(date != NULL)
		free(date);

	while(1)
	{
		/* Извлечение запросов клиентов из очереди */
		err = pthread_mutex_lock(&mutex);
		if (err != 0)
			handle_error("pthread_mutex_lock");
		
		while(iget == iput)
		{
			err = pthread_cond_wait(&cond, &mutex);
			if (err != 0)
				handle_error("pthread_cond_wait");
		}
		cfd = clifd[iget]; /* Connected socket to service */
		if(++iget == MAXNCLI)
			iget = 0;
		
		err = pthread_mutex_unlock(&mutex);
		if (err != 0)
			handle_error("pthread_mutex_unlock");

		bytes_recieved = recv(cfd, str_for_receive, DATE_STR_LEN, 0);
		if (bytes_recieved < 0)
			handle_error("ERROR reading from socket");
		else if(bytes_recieved == 0)
				printf("No data sended to server\n");
		log_info("bytes_recieved: %d", bytes_recieved);
		printf("Message from client: %s\n", str_for_receive);

		if((strncmp(str_for_receive, "Date", DATE_STR_LEN)) == 0)
		{
			bytes_sended = send(cfd, str_for_send, strlen(str_for_send) + 1, 0);
			if (bytes_sended < 0) 
				handle_error("ERROR send data to socket");
			log_info("bytes_sended: %d", bytes_sended);
		}
		else
			printf("Wrong request from client!\n");

		close(cfd);
		/* Loop to receive next client request */
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv) 
{
	log_info("Start of server program");
	int portno; // номер порта	
	pthread_t tid[SERVERS_NUM];
	memset(tid, 0, sizeof(tid));
	int result;
	int err = 0;
	int lfd, connfd; /* Listening socket, connected socket */

	if(argc != 2 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	portno = atoi(argv[1]);
	if(portno != 0 && portno < 1025 && portno > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера, клиента
	socklen_t clilen = sizeof(cli_addr);
	
	/* Создание сокета */
	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < 0)
		handle_error("error opening socket");
	 
	/* Связывание сокета с локальным адресом */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	/* Cервер принимает подключения на все адреса сетевых подключений сервера */
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* Вызываем bind для связывания */
	if (bind(lfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
			handle_error("error on binding");

	/* Ожидание подключений, размер очереди - QUEUE_SIZE */
	if (listen(lfd, QUEUE_SIZE) < 0)
		handle_error("listen() failed");

	int serving_threads_cnt = 0;
	for (; serving_threads_cnt < SERVERS_NUM; serving_threads_cnt++)
	{
		result = pthread_create(&tid[serving_threads_cnt], NULL, request_handler, NULL);
		if (result != 0) 
			handle_error("Creating thread");
		else 
			pthread_detach(tid[serving_threads_cnt]);
	}

	iget = iput = 0;
	while(1)
	{
		connfd = accept(lfd, (struct sockaddr *)&cli_addr, &clilen);
		if (connfd < 0)
			handle_error("error on accept");
		
		print_client_data(cli_addr);
		
		err = pthread_mutex_lock(&mutex);
		if(err != 0)
			handle_error("pthread_mutex_lock");

			clifd[iput] = connfd;
			if(++iput == MAXNCLI)
				iput = 0;
			if(iput == iget)
			{
				printf("iput = iget = %d\n", iput);
				exit(EXIT_FAILURE);
			}
			err = pthread_cond_signal(&cond);
			if (err != 0)
				handle_error("pthread_cond_signal");

		err = pthread_mutex_unlock(&mutex);
		if (err != 0)
			handle_error("pthread_mutex_unlock");

	}
	close(lfd);
	log_info("End of server program");
	exit(EXIT_SUCCESS);
}