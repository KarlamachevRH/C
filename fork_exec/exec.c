/* Задание.
Создать программу, порождающую дочерний процесс с помощью exec. */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>


int main(int argc, char* argv[]) 
{
	pid_t pid;

	/* Start child process */
	switch (pid = fork())
	{
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	
	case 0:
		if (execl("./test","test", NULL) < 0) 
		{
			perror("execl");
			exit(EXIT_FAILURE);
		}
		break;
	
	default:
		if(wait(NULL) == -1)
		{
			perror("wait");
			exit(EXIT_FAILURE);
		}
		printf("CHILD process pid=%d exited\n", pid);
		pid  = getpid();
		printf("PARENT process pid=%d exited\n", pid);
		exit(EXIT_SUCCESS);
	}
}