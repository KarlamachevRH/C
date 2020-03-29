/* 
 * For POSIX messages queue programs realization uncomment #define MSG_QUEUE_POSIX 
 * and comment #define MSG_QUEUE_SYSTEM_V in header.h file
 */

#include "headers.h"

int main(int argc, char **argv) 
{
	char str[MAX_BUF_SIZE];
	log_info("Start of server program");
	snprintf(str, MAX_BUF_SIZE, "%s", "Welcome to hell!");

#ifdef MSG_QUEUE_SYSTEM_V
	int msgqid, rc;
	long qtype1;
	long qtype2;
	key = ftok(path_name, proj_id);
	if((msgqid = msgget(key, IPC_CREAT | 0660)) == -1)
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}
	struct msgbuf qbuf;
	qtype1 = SERVER_MSG_TYPE;
	qtype2 = CLIENT_MSG_TYPE;
	send_message(msgqid, &qbuf, qtype1, str);
	read_message(msgqid, &qbuf, qtype2);
	if ((rc = msgctl(msgqid, IPC_RMID, NULL)) < 0)
	{
		perror("msgctl");
		log_info("msgctl (return queue) failed, rc=%d\n", rc);
		exit(EXIT_FAILURE);
	}
	log_info("End of server program");
	exit(EXIT_SUCCESS);
#endif // MSG_QUEUE_SYSTEM_V

#ifdef MSG_QUEUE_POSIX

	mqd_t mqid;
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_BUF_SIZE;
	attr.mq_curmsgs = 0;
	//unsigned int msg_pr;

	if((mqid = mq_open(name, O_CREAT | O_RDWR, 0666, &attr)) == -1)
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}

	send_message(mqid, str, MAX_BUF_SIZE, SERVER_MSG_PRIO);
	//read_message(mqid, str, MAX_BUF_SIZE, &msg_pr);

	if(mq_close(mqid) == -1)
	{
		perror("mq_close");
		exit(EXIT_FAILURE);
	}
	log_info("End of server program");
	exit(EXIT_SUCCESS);

#endif // MSG_QUEUE_POSIX
}