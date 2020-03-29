#include "headers.h"

#ifdef MSG_QUEUE_SYSTEM_V
void send_message(int qid, struct msgbuf *qbuf, long type, char *text)
{
	int msg_size = 0;
	qbuf->mtype = type;
	strncpy(qbuf->mtext, text, MAX_BUF_SIZE);
	msg_size = strlen(qbuf->mtext)+1;
	if(msg_size > MAX_BUF_SIZE)
	{
		printf("Message text too long\n");
		if((msgctl(qid, IPC_RMID, NULL)) < 0)
			perror("msgctl");
		exit(EXIT_FAILURE);
	}
	if((msgsnd(qid, qbuf, msg_size, 0)) == -1)
	{
		perror("msgsnd");
		if((msgctl(qid, IPC_RMID, NULL)) < 0)
			perror("msgctl");
		exit(EXIT_FAILURE);
	}
}
void read_message(int qid, struct msgbuf *qbuf, long type)
{
	qbuf->mtype = type;
	if(msgrcv(qid, qbuf, MAX_BUF_SIZE, type, 0) == -1)
	{
		perror("msgrcv");
		exit(EXIT_FAILURE);
	}
	printf("Type: %ld Message: %s\n", qbuf->mtype, qbuf->mtext);
}
#endif // MSG_QUEUE_SYSTEM_V

#ifdef MSG_QUEUE_POSIX
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
void read_message(mqd_t mqid, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
	if(mq_receive(mqid, msg_ptr, msg_len, msg_prio) == -1)
	{
		perror("mq_receive");
		if(mq_close(mqid) == -1)
			perror("mq_close");
		if(mq_unlink(name) == -1)
			perror("mq_unlink");
		exit(EXIT_FAILURE);
	}
	printf("Message: %s\n", msg_ptr);
}
#endif // MSG_QUEUE_POSIX