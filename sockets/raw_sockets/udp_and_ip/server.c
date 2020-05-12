#include "headers.h"

int udp_sockfd;

/* Печать адреса клиента */
void print_client_data(struct sockaddr_in recvd_server_addr)
{
	char server_addr[ADDR_LEN];
	printf("Client address: %s:%d\n", \
			inet_ntop(AF_INET, &recvd_server_addr.sin_addr.s_addr, server_addr, ADDR_LEN), \
			ntohs(recvd_server_addr.sin_port));
}

static void sig_handler(int sig)
{
	close(udp_sockfd);
#ifdef DEBUG
	printf("\n");
#endif //DEBUG
	log_info("End of server program");
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) 
{
	char recieving_buf[MAX_BUF_SIZE] = {0};
	char sending_buf[MAX_BUF_SIZE] = {0};
	int bytes_recieved = 0;
	int bytes_sended = 0;

	log_info("Start of server program");

	int port_num;
	struct sockaddr_in server_addr;
	struct sockaddr_in recieved_addr;

	if(argc != 2 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Извлечение порта */
	port_num = atoi(argv[1]);
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

	/* Заполнение структуры server_addr */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;

	/* Установка порта */
	server_addr.sin_port = htons(port_num);

	/* Вызываем bind для связывания */
	if (bind(udp_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
		handle_error("error on binding");

	socklen_t len = sizeof(struct sockaddr_in);
	bytes_recieved = recvfrom(udp_sockfd, recieving_buf, MAX_BUF_SIZE, 0, \
							 (struct sockaddr *)&recieved_addr, &len); 
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data recieved from client\n");
	log_info("bytes_recieved: %d", bytes_recieved);

	print_client_data(recieved_addr);
	printf("Message from client: %s\n", recieving_buf);

	snprintf(sending_buf, MAX_BUF_SIZE, "%s", "Hello from server!");

	bytes_sended = sendto(udp_sockfd, sending_buf, strlen(sending_buf) + 1, 0, \
						 (struct sockaddr *)&recieved_addr, sizeof(struct sockaddr_in));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);
	printf("Sended string to client: %s\n", sending_buf);

	/* Освободим сокет */
	close(udp_sockfd);

	log_info("End of server program");
	exit(EXIT_SUCCESS);
}