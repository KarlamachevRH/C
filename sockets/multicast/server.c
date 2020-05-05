#include "headers.h"

int udp_sockfd; // дескриптор сокета
char *date;

/* Печать адреса клиента */
void print_client_data(struct sockaddr_in cli_addr)
{
	char client_addr[CLIENT_ADDR_LEN];
	printf("Client address: %s:%d\n", \
			inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, client_addr, CLIENT_ADDR_LEN), \
			ntohs(cli_addr.sin_port));
}

/* Получим строку с текущей датой и временем */
char *get_date(char *date)
{
	struct tm *t = NULL;
	time_t timer;

	timer = time(NULL);
	if(timer == ((time_t)-1))
		handle_error("timer");

	t = localtime(&timer);
	if(t == NULL)
		handle_error("localtime");

	if((strftime(date, DATE_STR_LEN, "%d.%m.%Y %H:%M:%S, %A", t)) < 0)
	{
		printf("Can't get date\n");
		exit(EXIT_FAILURE);
	}
	return date;
}

static void sig_handler(int sig)
{
	close(udp_sockfd);
	if(date != NULL)
		free(date);
#ifdef DEBUG
	printf("\n");
#endif //DEBUG
	log_info("End of server program");
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) 
{
	log_info("Start of server program");

	int port_num; // номер порта
	int bytes_sended = 0;
	int ret;
	char *multicast_addr = NULL;

	if(argc != 3 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <multicast address> <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	multicast_addr = argv[1];

	port_num = atoi(argv[2]);
	if(port_num != 0 && port_num < 1025 && port_num > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	date = calloc(DATE_STR_LEN, sizeof(char));
	if(date == NULL)
		handle_error("calloc");
	
	struct sockaddr_in serv_addr; // структура сокета сервера
	
	/* Создание сокета */
	udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_sockfd < 0)
		handle_error("error opening socket");
	
	if ((signal(SIGINT, sig_handler)) == SIG_ERR)
	{
		perror("exit");
		exit(EXIT_FAILURE);
	}

	/* Заполнение структуры сокета сервера */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	ret = inet_aton(multicast_addr, (struct in_addr *)&serv_addr.sin_addr.s_addr);
	if(ret == 0)
	{
		printf("inet_aton: Unknown host address\n");
		exit(EXIT_FAILURE);
	}
	serv_addr.sin_port = htons(port_num);

	while (1)
	{
		get_date(date);
		bytes_sended = sendto(udp_sockfd, date, strlen(date) + 1, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
		if (bytes_sended < 0) 
			handle_error("ERROR send data to socket");
		log_info("bytes_sended: %d", bytes_sended);
		printf("Sended date string to multicast: %s\n", date);
		sleep(2);
	} /* While interrupt by Ctrl - C*/

	close(udp_sockfd);
	if(date != NULL)
		free(date);
	exit(EXIT_SUCCESS);
}