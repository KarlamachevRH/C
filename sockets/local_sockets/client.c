#include "headers.h"

int main(int argc, char **argv) 
{
	char send_buf[MAX_BUF_SIZE] = {0};
	char recieve_buf[MAX_BUF_SIZE] = {0};
	int bytes_sended = 0;
	int bytes_recieved = 0;
	char *serv_file_name = "serv";
	char *cli_file_name = "cli";
	
	log_info("Start of client program");
	snprintf(send_buf, MAX_BUF_SIZE, "%s", "Hello from client!");

	int sockfd;
	struct sockaddr_un serv_addr, cli_addr;

	/* Cоздание сокета */
	sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		handle_error("ERROR opening socket");
	
	/* Заполнение структуры serv_addr */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_LOCAL;
	strncpy(serv_addr.sun_path, serv_file_name, sizeof(serv_addr.sun_path)-1);

	/* Заполнение структуры cli_addr */
	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sun_family = AF_LOCAL;
	strncpy(cli_addr.sun_path, cli_file_name, sizeof(cli_addr.sun_path)-1);

	size_t size = sizeof(struct sockaddr_un);
	/* Вызываем bind для связывания */
	if (bind(sockfd, (struct sockaddr *)&cli_addr, size) < 0) 
		handle_error("error on binding");

	/* Чтение и передача сообщения */
	bytes_sended = sendto(sockfd, send_buf, strlen(send_buf) + 1, 0, \
						  (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_un));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);

	bytes_recieved = recvfrom(sockfd, recieve_buf, MAX_BUF_SIZE, 0, NULL, NULL); 
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data sended to client\n");
	log_info("bytes_recieved: %d", bytes_recieved);

	printf("Message from server: %s\n", recieve_buf);
	
	/* Освободим сокет */
	close(sockfd);

	if(remove(cli_file_name))
		handle_error("remove");

	log_info("End of client program");
	exit(EXIT_SUCCESS);
}