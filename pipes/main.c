#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <errno.h>
#include <string.h>

//#define DEBUG

#ifdef DEBUG
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(M, ...)
#endif //DEBUG

#define ARG_LEN_MAX 		32
#define ARG_NUM_MAX 		10
#define ARG_LIST_LEN_MAX 	128
#define PROG_LIST_LEN_MAX 	512
#define PROG_NUM_MAX 		12


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
	memset(programs_tokens, 1, sizeof(char*));
	char *arguments_tokens[ARG_NUM_MAX];
	memset(arguments_tokens, 1, sizeof(char*));	
	int programs_counter = 0;
	int arguments_counter = 0;
	char programs_list[PROG_LIST_LEN_MAX] = {0};
	char arguments_list[ARG_LIST_LEN_MAX] = {0};
	char *arguments[ARG_NUM_MAX];
	for(int i = 0; i < ARG_NUM_MAX; i++)
		arguments[i] = calloc(ARG_LEN_MAX, sizeof(char));
	int pipe_fd[2];
	
	if(pipe(pipe_fd) < 0)
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	printf(">");
	fgets(programs_list, PROG_LIST_LEN_MAX, stdin);
	int len = strlen(programs_list);
	programs_list[len-1] = 0; // '\n' = '\0'
	int i = 0;
	do
	{
		if(i == 0)
			programs_tokens[i] = strtok(programs_list, "|");
		else 
			programs_tokens[i] = strtok(NULL, "|");
		programs_counter++;
		i++;
	} while(programs_tokens[i-1] != NULL && programs_counter < PROG_NUM_MAX); // пока есть лексемы
	programs_counter--;
	log_info("programs_counter: %d", programs_counter);
	pid = calloc(programs_counter, sizeof(pid_t*));
	for(int i = 0; i < programs_counter; i++)
		pid[i] = calloc(1, sizeof(pid_t));

	for(int j = 0; j < programs_counter; j++)
	{
		switch (*pid[j] = fork())
		{
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
			break; /* We never reach this string, but I write it for good practice */
	
		case 0:
			strncpy(arguments_list, programs_tokens[j], ARG_LIST_LEN_MAX);
			log_info("arguments_list: %s", arguments_list);
			arguments_counter = 0;
			int i = 0;
			for(; arguments_tokens[i-1] != NULL && i < ARG_NUM_MAX; i++)
			{
				if(i == 0)
					arguments_tokens[i] = strtok(arguments_list, " ");
				else
					arguments_tokens[i] = strtok(NULL, " ");
				log_info("arguments_token: %s", arguments_tokens[i]);
				arguments_counter++;
			}
			arguments_tokens[i] = NULL;
			arguments_counter--;
			for(i = 0; i < arguments_counter; i++)
			{
				strncpy(arguments[i], arguments_tokens[i], ARG_LEN_MAX);
				log_info("argument #%d: %s", i, arguments[i]);
			}
			arguments[i] = NULL;
			/* 
			 * Заменим дескрипторы стандартного ввода/вывода на дескрипторы 
			 * pipe для передачи данных вывода одной программы в другую 
			 */
			if(j%2 > 0) //если четный номер итерации (первая из двух программ, обменивающихся данными по pipe)
			{
				close(pipe_fd[1]);
				dup2(pipe_fd[0], STDIN_FILENO);
			}
			else 	//если нечетный номер итерации (вторая из двух программ, обменивающихся данными по pipe)
			{
				close(pipe_fd[0]);
				dup2(pipe_fd[1], STDOUT_FILENO);
			}
			if (execvp(arguments[0], arguments) < 0)
			{
				perror("execvp");
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
	for(int i = 0; i < programs_counter; i++)
		if(pid[i])
			free(pid[i]);
	free(pid);
	for(int i = 0; i < ARG_NUM_MAX; i++)
		if(arguments[i])
				free(arguments[i]);
	log_info("End of program");
	exit(EXIT_SUCCESS);
}