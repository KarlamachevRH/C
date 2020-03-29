#include "headers.h"

int main(int argc, char **argv) 
{
	log_info("Start of client program");
	char str[MAX_BUF_SIZE];
	snprintf(str, MAX_BUF_SIZE, "%s", "Damn you!");

#ifdef MSG_QUEUE_SYSTEM_V
	int msgqid;
	long qtype1;
	long qtype2;
	key = ftok(path_name, proj_id);
	if((msgqid = msgget(key, 0)) == -1)
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}
	struct msgbuf qbuf;
	qtype1 = SERVER_MSG_TYPE;
	qtype2 = CLIENT_MSG_TYPE;
	send_message(msgqid, &qbuf, qtype2, str);
	read_message(msgqid, &qbuf, qtype1);	
	log_info("End of client program");
	exit(EXIT_SUCCESS);
#endif // MSG_QUEUE_SYSTEM_V

#ifdef MSG_QUEUE_POSIX
	mqd_t mqid;
	unsigned int msg_pr;
	if((mqid = mq_open(name, O_RDONLY)) == -1)
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}	
	//send_message(mqid, str, MAX_BUF_SIZE, CLIENT_MSG_PRIO);
	read_message(mqid, str, MAX_BUF_SIZE, &msg_pr);

	if(mq_close(mqid) == -1)
	{
		perror("mq_close");
		exit(EXIT_FAILURE);
	}
	if(mq_unlink(name) == -1)
	{
		perror("mq_unlink");
		exit(EXIT_FAILURE);
	}
	log_info("End of client program");
	exit(EXIT_SUCCESS);
#endif // MSG_QUEUE_POSIX
}