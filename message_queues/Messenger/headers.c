#include "headers.h"

void send_message(mqd_t mqid, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
	if(mq_send(mqid, msg_ptr, msg_len, msg_prio) == -1)
	{
		perror("mq_send");
		if(mq_close(mqid) == -1)
			perror("mq_close");
		if(mq_unlink(name) == -1)
			perror("mq_unlink");
		exit(EXIT_FAILURE);
	}
}
void read_message(mqd_t mqid, struct client_msg *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
	if(mq_receive(mqid, (char*)msg_ptr, msg_len, msg_prio) == -1)
	{
		perror("mq_receive");
		if(mq_close(mqid) == -1)
			perror("mq_close");
		if(mq_unlink(name) == -1)
			perror("mq_unlink");
		exit(EXIT_FAILURE);
	}
	printf("Client name: %s\nMessage: %s\n", msg_ptr->client_name, msg_ptr->msg);
}