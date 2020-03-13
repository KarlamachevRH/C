#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif //DEBUG

#define ARG_LEN_MAX 	32
#define ARG_NUM_MAX 	10
#define STR_LEN_MAX 	512
#define PROG_NUM_MAX 	12


int main(int argc, char **argv) 
{
	char *arg = NULL;
	if(argc > 1)
	{
		arg = argv[1];
		if(strncmp(arg, "-h", ARG_LEN_MAX) == 0 || 
		strncmp(arg, "--help", ARG_LEN_MAX) == 0)
		{
			printf("My shell programm. Working with pipes\n");
			printf("Usage: [program name 1] | [...] | [program name N]\n");
		}
	}
	log_info("Start of program");

	pid_t **pid = NULL;
	char *programs_tokens[PROG_NUM_MAX];
	char *arguments_tokens[ARG_NUM_MAX];
	int programs_counter = 0;
	int arguments_counter = 0;
	char programs_list[STR_LEN_MAX];
	char arguments_list[STR_LEN_MAX];
	int i = 0;
	int fd[2];
	
	if(pipe(fd) < 0)
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	printf(">");
	fgets(programs_list, STR_LEN_MAX, stdin);
	
	while (programs_tokens[i] != NULL && programs_counter < PROG_NUM_MAX)         // пока есть лексемы
	{
		programs_tokens[i] = strtok(programs_list, "|");
		programs_counter++;
	}

	pid = calloc(programs_counter, sizeof(pid_t*));
	for(i = 0; i < programs_counter; i++)
		pid[i] = calloc(1, sizeof(pid_t));

	for(i = 0; i < programs_counter; i++)
	{
		switch (*pid[i] = fork())
		{
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
			break; /* We never reach this string, but I write it for good practice */
	
		case 0:
			strncpy(arguments_list, programs_tokens[i], ARG_NUM_MAX);
			while (arguments_tokens[i] != NULL && programs_counter < ARG_NUM_MAX)
			{
				arguments_tokens[i] = strtok(arguments_list, " ");
				arguments_counter++;
			}
			arguments_list[0] = 0;
			for(i = 1; i < arguments_counter; i++)
			{
				strcat(arguments_list, "\"");
				strcat(arguments_list, arguments_tokens[i]);
				strcat(arguments_list, "\",");
			}
			/* 
			 * Заменим дескрипторы стандартного ввода/вывода на дескрипторы 
			 * pipe для передачи данных вывода одной программы в другую 
			 */
			if(i%2) 			//если четный номер итерации (первая из двух программ, обменивающихся данными по pipe)
				dup2(fd[1], 1);
			else 				//если нечетный номер итерации (вторая из двух программ, обменивающихся данными по pipe)
				dup2(fd[0], 0);
			
			if (execl("arguments_tokens[0]", arguments_list, NULL) < 0)
			{
				perror("execl");
				exit(EXIT_FAILURE);
			}
			break; 
		
		default:
			if(wait(NULL) == -1) // ждем пока завершится работа дочерней программы
			{
				perror("wait");
				exit(EXIT_FAILURE);
			}
			break;
		}
	}
	for(i = 0; i < programs_counter; i++)
		if(pid[i])
			free(pid[i]);
	free(pid);
	exit(EXIT_SUCCESS);
}