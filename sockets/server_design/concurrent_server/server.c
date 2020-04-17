#include "headers.h"
#include <sys/wait.h> 

/* Количество активных пользователей */
int nclients = 0;

/* Печать адреса клиента */
void print_client_data(struct sockaddr_in cli_addr)
{
	char client_addr[CLIENT_ADDR_LEN];
	printf("Client address: %s:%d\n", \
			inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, client_addr, CLIENT_ADDR_LEN), \
			ntohs(cli_addr.sin_port));
}

/* Получим строку с текущей датой */
char *get_date()
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

	if((strftime(s, DATE_STR_LEN, "%d.%m.%Y %H:%M:%S, %A", t)) > 0)
	{
		date = calloc(DATE_STR_LEN, sizeof(char));
		if(date == NULL)
			handle_error("calloc");
		strcpy(date, s);
	}
	else
	{
		printf("Can't get date\n");
		exit(EXIT_FAILURE);
	}
	return date;
}

/* SIGCHLD handler */
static void grimReaper(int sig)
{
	int savedErrno;	
	savedErrno = errno; /* waitpid() might change 'errno' */
	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;
	errno = savedErrno;
}

int main(int argc, char **argv) 
{
	log_info("Start of server program");

	int sockfd, newsockfd; // дескрипторы сокетов
	int portno; // номер порта
	int pid; // id номер процесса
	int bytes_sended = 0;
	int bytes_recieved = 0;
	char *date = NULL;
	char str_for_send[DATE_STR_LEN] = {0};
	char str_for_receive[DATE_STR_LEN] = {0};
	struct sigaction sa;

	if(argc != 2 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	date = get_date();

	portno = atoi(argv[1]);
	if(portno != 0 && portno < 1025 && portno > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	/* Establish SIGCHLD handler to reap terminated children */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = grimReaper;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		handle_error("sigaction");

	snprintf(str_for_send, DATE_STR_LEN, "%s", date);
	
	socklen_t clilen; // размер адреса клиента типа socklen_t
	struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента
	
	/* Создание сокета */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		handle_error("error opening socket");
	 
	/* Связывание сокета с локальным адресом */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	/* Cервер принимает подключения на все адреса сетевых подключений сервера */
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);

	/* Вызываем bind для связывания */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
			handle_error("error on binding");

	/* Ожидание подключений, размер очереди - QUEUE_SIZE */
	if (listen(sockfd, QUEUE_SIZE) < 0)
		handle_error("listen() failed");

	clilen = sizeof(cli_addr);
	
	while (1) 
	{
		/* Извлечение запросов на подключение из очереди */	
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			handle_error("error on accept");

		nclients++; // увеличиваем счетчик подключившихся клиентов
		
		print_client_data(cli_addr);

		pid = fork();
		if (pid < 0)
		{
			perror("ERROR on fork");
			break;
		}
		if (pid == 0) /* Child handles request */
		{
			close(sockfd);
			bytes_recieved = recv(newsockfd, str_for_receive, DATE_STR_LEN, 0);
			if (bytes_recieved < 0)
				handle_error("ERROR reading from socket");
			else if(bytes_recieved == 0)
					printf("No data sended to server\n");
			log_info("bytes_recieved: %d", bytes_recieved);
			printf("Message from client: %s\n", str_for_receive);

			if((strncmp(str_for_receive, "Date", DATE_STR_LEN)) == 0)
			{
				bytes_sended = send(newsockfd, str_for_send, strlen(str_for_send) + 1, 0);
				if (bytes_sended < 0) 
					handle_error("ERROR send data to socket");
				log_info("bytes_sended: %d", bytes_sended);
			}
			else
				printf("Wrong request from client!\n");
			close(newsockfd);
			exit(EXIT_SUCCESS);
		}
		else
			close(newsockfd);
		/* Parent loops to receive next client request */
	}
	close(sockfd);
	if(date != NULL)
		free(date);

	log_info("End of server program");
	exit(EXIT_SUCCESS);
}