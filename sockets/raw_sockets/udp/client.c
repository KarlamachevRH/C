#include "headers.h"

int udp_sockfd; // дескриптор сокета

/* Печать адреса клиента */
void print_server_data(struct sockaddr_in serv_addr)
{
	char server_addr[ADDR_LEN];
	printf("Server address: %s:%d\n", \
			inet_ntop(AF_INET, &serv_addr.sin_addr.s_addr, server_addr, ADDR_LEN), \
			ntohs(serv_addr.sin_port));
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
	log_info("Start of client program");

	int port_num; /* Номер порта */
	int bytes_sended = 0;
	int bytes_recieved = 0;
	struct sockaddr_in server_addr; /* Структура сокета сервера */
	struct sockaddr_in recieved_addr;
	struct hostent *server;
	char sending_buf[MAX_BUF_SIZE] = {0};
	char recieving_buf[MAX_BUF_SIZE] = {0};
	char *msg = "Hello from client!";

	struct udphdr udp_header; /* Заголовок протокола UDP для RAW - сокета */

	if(argc != 3 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <host name> <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port_num = atoi(argv[2]);
	if(port_num != 0 && port_num < 1025 && port_num > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}
	
	/* Создание сокета */
	udp_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (udp_sockfd < 0)
		handle_error("error opening socket");
	
	if ((signal(SIGINT, sig_handler)) == SIG_ERR)
	{
		perror("exit");
		exit(EXIT_FAILURE);
	}

	/* Извлечение хоста */
	server = gethostbyname(argv[1]);
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	/* Заполнение структуры адреса сервера */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memmove(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
	server_addr.sin_port = htons(port_num);

	/* Заполнение заголовка протокола UDP для RAW - сокета */
	udp_header.source = htons(port_num - 1);
	udp_header.dest = server_addr.sin_port;
	size_t payload_size = strlen(msg) + 1;
	size_t size = payload_size + sizeof(udp_header);
	udp_header.len = htons(size);
	udp_header.check = 0;

	memmove(sending_buf, &udp_header, sizeof(struct udphdr));
	char *p = sending_buf;
	p = p + sizeof(struct udphdr);
	snprintf(p, MAX_BUF_SIZE - sizeof(struct udphdr), "%s", msg);

	/* Отправка данных на сервер */
	bytes_sended = sendto(udp_sockfd, sending_buf, size, 0, \
						 (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);
	printf("Sended string to server: %s\n", p);
	
	socklen_t len = sizeof(struct sockaddr_in);

	/* 
	 * Сместим указатель в приемном буфере до адреса переменной, 
	 * в которой хранится порт назначения 
	 */
	uint16_t *rbp = (uint16_t *)&recieving_buf[0];
	rbp = rbp + sizeof(struct iphdr);
	rbp++;	
	
	while (1)
	{
		bytes_recieved = recvfrom(udp_sockfd, recieving_buf, MAX_BUF_SIZE, 0, \
								 (struct sockaddr *)&recieved_addr, &len);
		if (bytes_recieved < 0)
			handle_error("ERROR reading from socket");
		else if(bytes_recieved == 0)
				printf("No data recieved from server\n");
		log_info("bytes_recieved: %d", bytes_recieved);
		print_server_data(recieved_addr);

		/* 
		 * Порт назначения из UDP - заголовка в приемном буфере должен
		 * совпадать с портом отправителя, заполненном нами 
		 */
		if(*rbp == udp_header.source) 
		{
			rbp--;
			rbp = rbp + sizeof(struct udphdr);
			printf("Message from server: %s\n", (char*)rbp);
			break;
		}
	}

	close(udp_sockfd);
	log_info("End of client program");
	exit(EXIT_SUCCESS);
}