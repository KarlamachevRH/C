#include "fork_proccess.h"

int fork_programm(const char *name)
{
    pid_t pid;    
    int status, stat;    
    char procName[PATH_MAX] = {0};
    strncpy(procName, "./", PATH_MAX);
    strncat(procName, name, PATH_MAX);
    pid = fork();        
        if (pid == -1) 
        {
            perror("fork"); /* произошла ошибка */
            exit(EXIT_FAILURE); /*выход из родительского процесса*/
        } 
        else if (pid == 0) 
        {             
            if (execl(procName, name, NULL)<0) {
                printf("ERROR while start process\n");
                exit(EXIT_FAILURE);
		    }
            exit(EXIT_SUCCESS);
        }        
        status = waitpid(pid, &stat, 0);
        if (pid == status)
        {
            printf("Процесс-потомок %d завершен, result = %d\n", pid, WEXITSTATUS(stat));           
        }
        else 
        {
			 perror("PARENT: потомок не завершился успешно");             
		}
    return errno;
}