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
	
	log_info("Start of client program");

	int port_num;
	struct sockaddr_in serv_addr;
	struct ip_mreq mreq;
	char *multicast_addr = NULL;

	if(argc != 3 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <multicast address> <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	multicast_addr = argv[1];

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
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	/* Установка порта */
	serv_addr.sin_port = htons(port_num);

	/* Вызываем bind для связывания */
	if (bind(udp_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		handle_error("error on binding");

	/* Заполним структуру содержащую адрес мультикаст-группы, из которой необходимо получать данные */
	ret = inet_aton(multicast_addr, (struct in_addr *)&mreq.imr_multiaddr.s_addr);
	if(ret == 0)
	{
		printf("inet_aton: Unknown host address\n");
		exit(EXIT_FAILURE);
	}
	mreq.imr_interface.s_addr = INADDR_ANY;

	ret = setsockopt(udp_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if(ret < 0)
		handle_error("setsockopt multicast enable");

	socklen_t len = sizeof(struct sockaddr_in);
	while (1)
	{
		bytes_recieved = recvfrom(udp_sockfd, str_for_receive, DATE_STR_LEN, 0, (struct sockaddr *)&serv_addr, &len); 
		if (bytes_recieved < 0)
			handle_error("ERROR reading from socket");
		else if(bytes_recieved == 0)
				printf("No data recieved from server\n");
		log_info("bytes_recieved: %d", bytes_recieved);

		print_server_data(serv_addr);
		printf("Message from server: %s\n", str_for_receive);
	}

	/* Освободим сокет */
	close(udp_sockfd);

	log_info("End of client program");
	exit(EXIT_SUCCESS);
}