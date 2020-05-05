#include "headers.h"

int udp_sockfd;

/* Печать адреса клиента */
void print_server_data(struct sockaddr_in recvd_serv_addr)
{
	char server_addr[CLIENT_ADDR_LEN];
	printf("Server address: %s:%d\n", \
			inet_ntop(AF_INET, &recvd_serv_addr.sin_addr.s_addr, server_addr, CLIENT_ADDR_LEN), \
			ntohs(recvd_serv_addr.sin_port));
}

static void sig_handler(int sig)
{
	close(udp_sockfd);
#ifdef DEBUG
	printf("\n");
#endif //DEBUG
	log_info("End of client program");
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) 
{
	char str_for_receive[DATE_STR_LEN] = {0};
	int bytes_recieved = 0;
	int ret;
	char *broad_addr = NULL;
	
	log_info("Start of client program");

	int port_num;
	struct sockaddr_in serv_addr;

	if (argc < 3) 
	{
		fprintf(stderr,"Usage %s <broadcast address> <port>\n", argv[0]);
		exit(0);
	}

	broad_addr = argv[1];

	/* Извлечение порта */
	port_num = atoi(argv[2]);
	if(port_num != 0 && port_num < 1025 && port_num > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	/* Cоздание сокета */
	udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_sockfd < 0) 
		handle_error("ERROR opening socket");

	if ((signal(SIGINT, sig_handler)) == SIG_ERR)
	{
		perror("exit");
		exit(EXIT_FAILURE);
	}

	/* Заполнение структуры serv_addr */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	ret = inet_aton(broad_addr, (struct in_addr *)&serv_addr.sin_addr.s_addr);
	if(ret == 0)
	{
		printf("inet_aton: Unknown host address\n");
		exit(EXIT_FAILURE);
	}
	/* Установка порта */
	serv_addr.sin_port = htons(port_num);

	/* Вызываем bind для связывания с широковещательным адресом */
	if (bind(udp_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		handle_error("error on binding");

	socklen_t len = sizeof(struct sockaddr_in);
	while (1)
	{
		bytes_recieved = recvfrom(udp_sockfd, str_for_receive, DATE_STR_LEN, 0, (struct sockaddr *)&serv_addr, &len); 
		if (bytes_recieved < 0)
			handle_error("ERROR reading from socket");
		else if(bytes_recieved == 0)
				printf("No data sended to client\n");
		log_info("bytes_recieved: %d", bytes_recieved);

		print_server_data(serv_addr);
		printf("Message from server: %s\n", str_for_receive);
	}

	/* Освободим сокет */
	close(udp_sockfd);

	log_info("End of client program");
	exit(EXIT_SUCCESS);
}
