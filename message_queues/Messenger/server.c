#include "headers.h"

static mqd_t mqid;

static void sig_handler(int sig)
{
	if(mq_close(mqid) == -1)
	{
		perror("mq_close");
		exit(EXIT_FAILURE);
	}
	/* Unlink message queue here for delete after all clients will close his own queue descriptors */
	if(mq_unlink(name) == -1) 
	{
		perror("mq_unlink");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG
	printf("\n");
#endif //DEBUG
	log_info("End of server program");
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) 
{
	log_info("Start of server program");
	struct client_msg msg, *msg_ptr;
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(struct client_msg);
	attr.mq_curmsgs = 0;

	if((mqid = mq_open(name, O_CREAT | O_RDWR, 0666, &attr)) == -1)
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}

	if ((signal(SIGINT, sig_handler)) == SIG_ERR)
	{
		perror("exit");
		exit(EXIT_FAILURE);
	}

	msg_ptr = &msg;

	while(1)
	{
		read_message(mqid, msg_ptr, sizeof(struct client_msg), NULL);
	}
	
	log_info("End of server program");
	exit(EXIT_SUCCESS);
}