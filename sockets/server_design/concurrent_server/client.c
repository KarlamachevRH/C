#include "headers.h"
#include <pthread.h>

#define CLIENTS_NUM 10

struct request_data
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int clientsnum;
	struct sockaddr_in *serv_addr;
}data= 
{ 
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	0,
	NULL
};

void *request(void *arg)
{
	int sockfd;
	int bytes_sended = 0;
	int bytes_recieved = 0;
	char str_for_send[DATE_STR_LEN] = {0};
	char str_for_receive[DATE_STR_LEN] = {0};
	pthread_t th = pthread_self();
	int err;

	snprintf(str_for_send, DATE_STR_LEN, "%s", "Date");

	/* Cоздание сокета */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		handle_error("ERROR opening socket");

	/* Установка соединения */
	if(connect(sockfd,(struct sockaddr *)data.serv_addr, sizeof(struct sockaddr_in)) < 0)
		handle_error("ERROR connecting");
	log_info("Connect to server established");
	
	/* Чтение и передача сообщения */
	bytes_sended = send(sockfd, str_for_send, strlen(str_for_send) + 1, 0); // отправка данных
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);

	bytes_recieved = recv(sockfd, str_for_receive, DATE_STR_LEN, 0); // чтение данных
	if (bytes_recieved < 0) 
		handle_error("ERROR reading from socket");
	log_info("bytes_recieved: %d", bytes_recieved);

	err = pthread_mutex_lock(&data.mutex);
	if (err != 0)
		handle_error("pthread_mutex_lock");

		data.clientsnum++;
		printf("Thread id: %ld, #%d\n", th, data.clientsnum);
		printf("Message from server: %s\n", str_for_receive);
		if(data.clientsnum == CLIENTS_NUM)
		{
			err = pthread_cond_signal(&data.cond);
			if (err != 0)
				handle_error("pthread_cond_signal");
		}

	err = pthread_mutex_unlock(&data.mutex);
	if (err != 0)
		handle_error("pthread_mutex_unlock");

	/* Освободим сокет */
	close(sockfd);
	pthread_exit(NULL);
}

int main(int argc, char **argv) 
{
	pthread_t tid[CLIENTS_NUM];
	int result;

	log_info("Start of client program");
	
	int portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int err;

	if (argc < 3) 
	{
		fprintf(stderr,"Usage %s <hostname> <port(1025-65535)>\n", argv[0]);
		exit(0);
	}

	/* Извлечение порта */
	portno = atoi(argv[2]);
	if(portno != 0 && portno < 1025 && portno > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

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
	data.serv_addr = &serv_addr;

	for (int i = 0; i < CLIENTS_NUM; i++)
	{
		result = pthread_create(&tid[i], NULL, request, NULL);
		if (result != 0) 
			handle_error("Creating thread");
		else 
			pthread_detach(tid[i]);
	}
	err = pthread_mutex_lock(&data.mutex);
	if (err != 0)
		handle_error("pthread_mutex_lock");

		err = pthread_cond_wait(&data.cond, &data.mutex);		
		if (err != 0)
			handle_error("pthread_mutex_unlock");
		printf("Client is ended work. Press \"Enter\" to quit program\n");

	err = pthread_mutex_unlock(&data.mutex);
	if (err != 0)
		handle_error("pthread_mutex_unlock");
	
	getchar();
	log_info("End of client program");
	exit(EXIT_SUCCESS);
}