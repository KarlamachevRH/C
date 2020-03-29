#include "headers.h"

int main(int argc, char **argv) 
{
	if(argc < 2)
	{
		printf("Usage: %s [Client name]", argv[0]);
		exit(EXIT_FAILURE);
	}
	log_info("Start of client program");
	char str[MAX_BUF_SIZE];
	char *client_name = argv[1];
	struct client_msg msg, *msg_ptr;
	mqd_t mqid;
	
	/* Filling message structure for send to server */
	snprintf(str, MAX_BUF_SIZE, "this is client's %s message", client_name);
	printf("%s", str);
	strncpy(msg.client_name, client_name, NAME_LEN);
	strncpy(msg.msg, str, MSG_LEN);
	msg_ptr = &msg;

	if((mqid = mq_open(name, O_RDWR)) == -1)
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}

	send_message(mqid, (char*)msg_ptr, sizeof(struct client_msg), 0);
	
	if(mq_close(mqid) == -1)
	{
		perror("mq_close");
		exit(EXIT_FAILURE);
	}
	log_info("End of client program");
	exit(EXIT_SUCCESS);
}