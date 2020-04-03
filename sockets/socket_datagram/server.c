#include "headers.h"


/* Печать адреса клиента */
void print_client_data(struct sockaddr_in cli_addr)
{
	char client_addr[CLIENT_ADDR_LEN];
	printf("Client address: %s:%d\n", \
			inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, client_addr, CLIENT_ADDR_LEN), \
			ntohs(cli_addr.sin_port));
}

int main(int argc, char **argv) 
{
	log_info("Start of server program");

	int sockfd; // дескрипторы сокетов
	int portno; // номер порта
	int bytes_sended = 0;
	int bytes_recieved = 0;

	if(argc != 2 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <port number (range: 1024 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	portno = atoi(argv[1]);
	if(portno != 0 && portno < 1025 && portno > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	char str_for_send[MAX_BUF_SIZE] = {0};
	char str_for_receive[MAX_BUF_SIZE] = {0};

	snprintf(str_for_send, MAX_BUF_SIZE, "%s", "Hello from server!");
	
	struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента
	
	/* Создание сокета */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		handle_error("error opening socket");
	 
	/* Связывание сокета с локальным адресом */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	/* Cервер принимает подключения на все адреса сетевых подключений сервера */
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);

	/* Вызываем bind для связывания */
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
			handle_error("error on binding");
	
	socklen_t len = sizeof(struct sockaddr_in);
	bytes_recieved = recvfrom(sockfd, str_for_receive, MAX_BUF_SIZE, 0, (struct sockaddr *)&cli_addr, &len); 
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data sended to server\n");

	log_info("bytes_recieved: %d", bytes_recieved);

	print_client_data(cli_addr);
	printf("Message from client: %s\n", str_for_receive);

	bytes_sended = sendto(sockfd, str_for_send, strlen(str_for_send) + 1, 0, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);

	close(sockfd);

	log_info("End of server program");
	exit(EXIT_SUCCESS);
}