#include "headers.h"

#ifdef POLL
#include <poll.h>
#endif // POLL

#ifdef EPOLL
#include <sys/epoll.h>
#endif // EPOLL

#define FD_NUM 2

enum protocol_type
{
	UDP,
	TCP
};

/* Печать адреса клиента */
void print_client_data(struct sockaddr_in cli_addr)
{
	char client_addr[CLIENT_ADDR_LEN];
	printf("Client address: %s:%d\n", \
			inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, client_addr, CLIENT_ADDR_LEN), \
			ntohs(cli_addr.sin_port));
}

int create_socket(int SOCK_TYPE)
{
	int sock_fd;
	/* Создание сокета */
	sock_fd = socket(AF_INET, SOCK_TYPE, 0);
	if (sock_fd < 0)
		handle_error("error opening socket");
	return sock_fd;
}

int max(int *fds, int n)
{
	int max = fds[0];
	for(int i = 1; i < n; i++)
	{
		if(fds[i] > max)
			max=fds[i];
	}
	return max;
}

void set_server_addr(struct sockaddr_in *serv_addr, int port_num)
{
	memset(serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr->sin_family = AF_INET;
	/* Cервер принимает подключения на все адреса сетевых подключений сервера */
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons(port_num);
}

void handle_request_from_UDP_socket(int udp_sock_fd)
{
	int bytes_sended = 0;
	int bytes_recieved = 0;
	socklen_t len = sizeof(struct sockaddr_in);
	struct sockaddr_in cli_addr;

	char str_for_send[MAX_BUF_SIZE] = {0};
	snprintf(str_for_send, MAX_BUF_SIZE, "%s", "Hello from server!");
	char str_for_receive[MAX_BUF_SIZE] = {0};

	bytes_recieved = recvfrom(udp_sock_fd, str_for_receive, MAX_BUF_SIZE, 0, \
							  (struct sockaddr *)&cli_addr, &len); 
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data sended to server\n");

	log_info("bytes_recieved: %d", bytes_recieved);

	print_client_data(cli_addr);
	printf("Message from client: %s\n", str_for_receive);

	bytes_sended = sendto(udp_sock_fd, str_for_send, strlen(str_for_send) + 1, 0, \
						  (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);
}

void handle_request_from_TCP_socket(int tcp_sock_fd)
{
	int bytes_sended = 0;
	int bytes_recieved = 0;
	int newsockfd;
	socklen_t len = sizeof(struct sockaddr_in);
	struct sockaddr_in cli_addr;

	char str_for_send[MAX_BUF_SIZE] = {0};
	snprintf(str_for_send, MAX_BUF_SIZE, "%s", "Hello from server!");
	char str_for_receive[MAX_BUF_SIZE] = {0};
	
	/* Извлечение запросов на подключение из очереди */	
	newsockfd = accept(tcp_sock_fd,(struct sockaddr *) &cli_addr, &len);
	if (newsockfd < 0)
		handle_error("error on accept");
	
	print_client_data(cli_addr);

	bytes_recieved = recv(newsockfd, str_for_receive, MAX_BUF_SIZE, 0);
	if (bytes_recieved < 0)
		handle_error("ERROR reading from socket");
	else if(bytes_recieved == 0)
			printf("No data sended to server\n");
	
	log_info("bytes_recieved: %d", bytes_recieved);
	printf("Message from client: %s\n", str_for_receive);

	bytes_sended = send(newsockfd, str_for_send, strlen(str_for_send) + 1, 0);
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");

	log_info("bytes_sended: %d", bytes_sended);
	close(newsockfd);
}

int main(int argc, char **argv) 
{
	log_info("Start of server program");

	int udp_sock_fd, tcp_sock_fd; // дескрипторы сокетов
	int udp_port_num, tcp_port_num; // номера портов
	int prot_type = UDP;
	int ret;

	if(argc != 3 || strcmp(argv[1], "--help") == 0)
	{
		printf("Usage: %s <udp port number(1025 - 65535)>", argv[0]);
		printf(" <tcp port number(1025 - 65535, exclude udp port number)>\n");
		exit(EXIT_FAILURE);
	}

	udp_port_num = atoi(argv[1]);
	if(udp_port_num != 0 && udp_port_num < 1025 && udp_port_num > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}

	tcp_port_num = atoi(argv[2]);
	if(tcp_port_num != 0 && tcp_port_num < 1025 && tcp_port_num > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}
	
	struct sockaddr_in udp_serv_addr, tcp_serv_addr;
	
	udp_sock_fd = create_socket(SOCK_DGRAM);
	tcp_sock_fd = create_socket(SOCK_STREAM);

	set_server_addr(&udp_serv_addr, udp_port_num);
	set_server_addr(&tcp_serv_addr, tcp_port_num);

	/* Вызываем bind для связывания */
	if (bind(udp_sock_fd, (struct sockaddr *)&udp_serv_addr, sizeof(udp_serv_addr)) < 0) 
		handle_error("error on binding UDP socket");
	if (bind(tcp_sock_fd, (struct sockaddr *)&tcp_serv_addr, sizeof(tcp_serv_addr)) < 0) 
		handle_error("error on binding TCP socket");
	if (listen(tcp_sock_fd, QUEUE_SIZE) < 0)
		handle_error("listen() failed");

#ifdef SELECT

	fd_set readfds;
	int fds[FD_NUM];
	int maxfdp1;

	FD_ZERO(&readfds);

	FD_SET(udp_sock_fd, &readfds);
	FD_SET(tcp_sock_fd, &readfds);

	fds[UDP] = udp_sock_fd;
	fds[TCP] = tcp_sock_fd;
	maxfdp1 = max(fds, FD_NUM) + 1; /* max fd number plus 1*/
	log_info("maxfdp1 = %d", maxfdp1);
	
	ret = select(maxfdp1, &readfds, NULL, NULL, NULL);
	if(ret == -1)
		handle_error("select");

	if((FD_ISSET(udp_sock_fd, &readfds)))
		prot_type = UDP;
	else if((FD_ISSET(tcp_sock_fd, &readfds)))
		prot_type = TCP;
	log_info("prot_type = %d", prot_type);

#endif // SELECT

#ifdef POLL

	struct pollfd pollFd[FD_NUM];
	pollFd[UDP].fd = udp_sock_fd;
	pollFd[TCP].fd = tcp_sock_fd;

	pollFd[UDP].events = POLLIN;
	pollFd[TCP].events = POLLIN;
	
	ret = poll(pollFd, FD_NUM, -1);
	if(ret == -1)
		handle_error("poll");

	if(pollFd[UDP].revents & POLLIN)
		prot_type = UDP;
	
	if(pollFd[TCP].revents & POLLIN)
		prot_type = TCP;


#endif // POLL


#ifdef EPOLL

	int epfd;
	struct epoll_event ev[FD_NUM];
	struct epoll_event evlist[FD_NUM];

	epfd = epoll_create(FD_NUM);
	if (epfd == -1)
		handle_error("epoll_create");
	
	ev[UDP].events = EPOLLIN;
	ev[UDP].data.fd = udp_sock_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, udp_sock_fd, &ev[UDP]) == -1)
		handle_error("epoll_ctl");
	
	ev[TCP].events = EPOLLIN;
	ev[TCP].data.fd = tcp_sock_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, tcp_sock_fd, &ev[TCP]) == -1)
		handle_error("epoll_ctl");
	
	ret = epoll_wait(epfd, evlist, FD_NUM, -1);
	if (ret == -1)
		handle_error("epoll_wait");
	
	for (int j = 0; j < ret; j++)
	{
		if (evlist[j].events & EPOLLIN)
		{
			if(evlist[j].data.fd == udp_sock_fd)
				prot_type = UDP;
			if(evlist[j].data.fd == tcp_sock_fd)
				prot_type = TCP;
		}
	}

#endif // EPOLL
	
	switch (prot_type)
	{
	case UDP:
		handle_request_from_UDP_socket(udp_sock_fd);
		break;
	case TCP:
		handle_request_from_TCP_socket(tcp_sock_fd);
		break;
	default:
		break;
	}

	close(udp_sock_fd);
	close(tcp_sock_fd);

	log_info("End of server program");
	exit(EXIT_SUCCESS);
}