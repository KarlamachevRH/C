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


int main(int argc, char* argv[]) 
{
    int status, stat;
	pid_t pid, ppid;

	// запускаем дочерний процесс
	pid = fork();
	if (pid == -1)
	{
		perror("fork"); /* произошла ошибка */
		exit(EXIT_FAILURE); /*выход из родительского процесса*/
	} 
	else 
		if (pid == 0) 
		{
			ppid = getppid();
			pid = getpid();
			printf("CHILD: ppid: %d\n", ppid);
			printf("CHILD: pid: %d\n", pid);
			exit(EXIT_SUCCESS); /* выход из процесс-потомока */
		}
    
    // если выполняется родительский процесс
	ppid = getppid();
	pid = getpid();
	printf("PARENT: ppid: %d\n", ppid);
	printf("PARENT: pid: %d\n", pid);
    // ожидание окончания выполнения запущенного процесса
	status = waitpid(pid, &stat, 0);
	if (pid == status)
		printf("Процесс-потомок завершен, status = %d\n", WEXITSTATUS(stat));

    return EXIT_SUCCESS;
}