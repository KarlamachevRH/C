#include "headers.h"

/* Печать адреса сервера */
void print_server_data(struct sockaddr_in recvd_serv_addr)
{
	char server_addr[CLIENT_ADDR_LEN];
	printf("Server address: %s:%d\n", \
			inet_ntop(AF_INET, &recvd_serv_addr.sin_addr.s_addr, server_addr, CLIENT_ADDR_LEN), \
			ntohs(recvd_serv_addr.sin_port));
}

int main(int argc, char **argv) 
{
	char str_for_send[MAX_BUF_SIZE] = {0};
	char str_for_receive[MAX_BUF_SIZE] = {0};
	int bytes_sended = 0;
	int bytes_recieved = 0;
	
	log_info("Start of client program");
	snprintf(str_for_send, MAX_BUF_SIZE, "%s", "Hello from client!");

	int my_sock, portno;
	struct sockaddr_in serv_addr, recvd_serv_addr;
	struct hostent *server;

	if (argc < 3) 
	{
		fprintf(stderr,"Usage %s <hostname> <port>\n", argv[0]);
		exit(0);
	}  

	/* Извлечение порта */
	portno = atoi(argv[2]);
	if(portno != 0 && portno < 1025 && portno > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	/* Cоздание сокета */
	my_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (my_sock < 0) 
		handle_error("ERROR opening socket");

	/* Извлечение хоста */
	server = gethostbyname(argv[1]);
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	/* Заполнение структуры serv_addr */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memmove(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

	/* Установка порта */
	serv_addr.sin_port = htons(portno);

	/* Чтение и передача сообщения */
	bytes_sended = sendto(my_sock, str_for_send, strlen(str_for_send) + 1, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);

	socklen_t len = sizeof(struct sockaddr_in);
	bytes_recieved = recvfrom(my_sock, str_for_receive, MAX_BUF_SIZE, 0, (struct sockaddr *)&recvd_serv_addr, &len); 
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data sended to client\n");
	log_info("bytes_recieved: %d", bytes_recieved);

	print_server_data(recvd_serv_addr);
	printf("Message from server: %s\n", str_for_receive);
	
	/* Освободим сокет */
	close(my_sock);

	log_info("End of client program");
	exit(EXIT_SUCCESS);
}