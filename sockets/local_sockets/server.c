#include "headers.h"

int main(int argc, char **argv) 
{
	log_info("Start of server program");

	int sockfd; // дескриптор сокета
	int bytes_sended = 0;
	int bytes_recieved = 0;
	size_t size = 0;

	char *file_name = "serv";

	char send_buf[MAX_BUF_SIZE] = {0};
	char recieve_buf[MAX_BUF_SIZE] = {0};

	snprintf(send_buf, MAX_BUF_SIZE, "%s", "Hello from server!");
	
	struct sockaddr_un serv_addr, cli_addr;
	
	/* Создание сокета */
	sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (sockfd < 0)
		handle_error("error opening socket");
	 
	/* Связывание сокета */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_LOCAL;
	strncpy(serv_addr.sun_path, file_name, sizeof(serv_addr.sun_path)-1);

	size = sizeof(struct sockaddr_un);
	/* Вызываем bind для связывания */
	if (bind(sockfd, (struct sockaddr *)&serv_addr, size) < 0) 
		handle_error("error on binding");
	
	socklen_t len = sizeof(struct sockaddr_un);
	bytes_recieved = recvfrom(sockfd, recieve_buf, MAX_BUF_SIZE, 0, (struct sockaddr *)&cli_addr, &len); 
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data sended to server\n");

	log_info("bytes_recieved: %d", bytes_recieved);

	printf("Message from client: %s\n", recieve_buf);

	bytes_sended = sendto(sockfd, send_buf, strlen(send_buf) + 1, 0, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_un));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);

	close(sockfd);
	if(remove(file_name))
		handle_error("remove");

	log_info("End of server program");
	exit(EXIT_SUCCESS);
}