/* ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ.
1. Создать программу, порождающую дочерний процесс. Вывести pid и ppid родителя и потомка.
2. Создать иерархию из 6 процессов. Вывести pid и ppid родителей и потомков. */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>

#define NUM_PROC 2
#define TREE_LEVEL_MAX 2

enum PROC_TYPE
{
	LEFT_PROC,
	RIGHT_PROC
};


/* Если выполняется родительский процесс */
void parent_proccess(int num_proccesses, pid_t *pid)
{
	int i, status, stat;
	pid_t getPid, getPpid;
	getPpid = getppid();
	getPid = getpid();
	printf("PARENT: ppid: %d\n", getPpid);
	printf("PARENT: pid: %d\n", getPid);
	// ожидание окончания выполнения запущенных процессов
	for(i = 0; i < num_proccesses; i++)
	{
		status = waitpid(pid[i], &stat, 0);
		if (pid[i] == status)
			printf("Процесс-потомок %d завершен, status = %d\n", i, WEXITSTATUS(stat));
	}
}

/* Запускаем дочерний процесс */
void fork_proccess(int num)
{
	pid_t pid, ppid;
	int num_processes = 1;
	pid = fork();
	if (pid == -1)
	{
		perror("fork"); /* произошла ошибка */
		exit(EXIT_FAILURE); /*выход из родительского процесса*/
	} 
	else 
		if (pid == 0) 
		{
			num++;
			ppid = getppid();
			pid = getpid();
			printf("CHILD: ppid: %d\n", ppid);
			printf("CHILD: pid: %d\n", pid);
			if(num < TREE_LEVEL_MAX)
				fork_proccess(num);
			exit(EXIT_SUCCESS); /* выход из процесс-потомока */
		}
	parent_proccess(num_processes, &pid);
}

/* Запускаем 2 дочерниx процесса */
void fork_proccesses(int num)
{
	pid_t pid[NUM_PROC], getPid, getPpid;

	for (int i = 0; i < NUM_PROC; i++) 
	{
		// запускаем дочерние процессы
		pid[i] = fork();
		if (pid[i] == -1) 
		{
			perror("fork"); /* произошла ошибка */
			exit(EXIT_FAILURE); /*выход из родительского процесса*/
		} 
		else 
			if (pid[i] == 0) 
			{
				num++;
				getPpid = getppid();
				getPid = getpid();
				printf("CHILD: ppid: %d\n", getPpid);
				printf("CHILD: pid: %d\n", getPid);
				if(num < TREE_LEVEL_MAX && i == LEFT_PROC)
				{
					fork_proccess(num);
				}
				else 
				if(num < TREE_LEVEL_MAX && i == RIGHT_PROC)
				{
					fork_proccesses(num);
				}
				exit(EXIT_SUCCESS); /* выход из процесс-потомока */
			}
	}
	parent_proccess(NUM_PROC, pid);
}

int main(int argc, char* argv[]) 
{
	int num = 0;
	fork_proccesses(num);
	return EXIT_SUCCESS;
}