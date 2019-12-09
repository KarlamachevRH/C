/* ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ.
1. Модифицировать и отладить в соответствии с вариантом и выбранным средством коммуникации программу из лабораторной работы №6, реализующую порожденный процесс. При необходимости модифицировать ее в отдельную программу-сервер.
2. Модифицировать и отладить в соответствии с вариантом и выбранным средством коммуникации программу из лабораторной работы №6, реализующую родительский процесс. При необходимости модифицировать ее в отдельную программу-клиент.
   ВАРИАНТ ЗАДАНИЯ.
2. Умножение матрицы на вектор. Обработка одной строки матрицы - в порожденном процессе. */

#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/errno.h>

#define MAX_SEND_SIZE 80
#define N 3
#define M 4

struct mymsgbuf {
        long mtype;
        char mtext[MAX_SEND_SIZE];
};

int msgqid, rc;
void send_message(int qid, struct mymsgbuf *qbuf, long type, char *text);//отправить данные
void read_message(int qid, struct mymsgbuf *qbuf, long type);//принять данные
int matrix_str_multiplication(int *matrixStr, int *vector); //Функция умножения строки матрицы на вектор-столбец

int main(int argc, char* argv[]) {
	int matrix[M][N] =
    {{8, 3, 9}, 
     {2, 5, 7}, 
     {8, 8, 9},
     {7, 3, 2}};     
    int vector[N] = 
    {4, 
     5, 
     9};
    int i, pid[M], status, stat;

    key_t key;
    int qtype = 1;    
    struct mymsgbuf qbuf;

    key = ftok(".", 'm');
	if((msgqid = msgget(key, IPC_CREAT|0660)) == -1) {
		perror("msgget");
		exit(1);
	}

    for (i = 0; i < M; i++) {
        // запускаем дочерний процесс 
        pid[i] = fork();        
        if (-1 == pid[i]) {
            perror("fork"); /* произошла ошибка */
            exit(1); /*выход из родительского процесса*/
        } else if (0 == pid[i]) {
            printf(" CHILD: Это %d процесс-потомок!\n", i);
            
            char str[10];
            sprintf(str, "%d", matrix_str_multiplication(matrix[i], vector));

            send_message(msgqid, (struct mymsgbuf *)&qbuf, qtype, str); 
			printf(" CHILD: Это %d процесс-потомок отправил сообщение!\n", i);
			fflush(stdout);
            exit(0); /* выход из процесс-потомока */
        }
    }    
    // если выполняется родительский процесс
    printf("PARENT: Это процесс-родитель!\n");
    int resultVector[M];
    // ожидание окончания выполнения всех запущенных процессов
    for (i = 0; i < M; i++) {
        status = waitpid(pid[i], &stat, 0);
        if (pid[i] == status) {
            printf("Процесс-потомок %d завершен, status = %d\n", i, WEXITSTATUS(stat));
            read_message(msgqid, &qbuf, qtype);
            resultVector[i] = atoi(qbuf.mtext);                       
        }
    }
    printf("Значения аргументов вектора - результата умножения:\n");
    for (i = 0; i < M; i++) {                      
        printf("%d \n", resultVector[i]);           
    } 
    if ((rc = msgctl(msgqid, IPC_RMID, NULL)) < 0) {
		perror( strerror(errno) );
		printf("msgctl (return queue) failed, rc=%d\n", rc);
		return 1;
	}   
    return 0;   
}

void send_message(int qid, struct mymsgbuf *qbuf, long type, char *text){
        qbuf->mtype = type;
        strcpy(qbuf->mtext, text);

        if((msgsnd(qid, (struct msgbuf *)qbuf,
                strlen(qbuf->mtext)+1, 0)) ==-1){
                perror("msgsnd");
                exit(1);
        }
}

void read_message(int qid, struct mymsgbuf *qbuf, long type){
        qbuf->mtype = type;
        msgrcv(qid, (struct msgbuf *)qbuf, MAX_SEND_SIZE, type, 0);
        //printf("Type: %ld Text: %s\n", qbuf->mtype, qbuf->mtext);
}

int matrix_str_multiplication(int *matrixStr, int *vector){
    int newVecValue = 0;
    int mult = 1;
    for(int i = 0; i < N; i++){
        mult = matrixStr[i] * vector[i];
        newVecValue += mult;
    }
    return newVecValue;
}