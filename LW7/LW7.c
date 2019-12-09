/*  ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ. 
1. В соответствии с особенностями реализации варианта задания выбрать средство реализации межзадачных коммуникаций.
2. Модифицировать и отладить в соответствии с вариантом и выбранным средством коммуникации программу из лабораторной работы №6, реализующую порожденный процесс. При необходимости модифицировать ее в отдельную программу-сервер.
3. Модифицировать и отладить в соответствии с вариантом и выбранным средством коммуникации программу из лабораторной работы №6, реализующую родительский процесс. При необходимости модифицировать ее в отдельную программу-клиент. 
	ВАРИАНТ ЗАДАНИЯ.
2. Умножение матрицы на вектор. Обработка одной строки матрицы - в порожденном процессе. */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

#define N 3
#define M 4

int matrix_str_multiplication(int *matrixStr, int *vector); //Функция умножения строки матрицы на вектор-столбец

int main(void) {
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
    int fd[M][2];	
    
    for (i = 0; i < M; i++) {
        // запускаем дочерний процесс 
        pipe(fd[i]); 
        pid[i] = fork();        
        if (-1 == pid[i]) {
            perror("fork"); /* произошла ошибка */
            exit(1); /*выход из родительского процесса*/
        } else if (0 == pid[i]) {
            //printf(" CHILD: Это %d процесс-потомок!\n", i);  
            /* процесс-потомок закрывает доступный для чтения конец канала 0*/
            close(fd[i][0]);          
            int newVecValue = matrix_str_multiplication(matrix[i], vector); 
            write(fd[i][1], &newVecValue, sizeof(int));
            exit(0);            
        }
    }
    // если выполняется родительский процесс
    printf("PARENT: Это процесс-родитель!\n");
    // ожидание окончания выполнения всех запущенных процессов
    for (i = 0; i < M; i++) {
        status = waitpid(pid[i], &stat, 0);
        if (pid[i] == status) {
			printf("Процесс-потомок %d завершен,  exit status = %d\n", i, WEXITSTATUS(stat));
			/* процесс-родитель закрывает доступный для записи конец канала 1*/
			close(fd[i][1]);
			/* читает из канала 0*/
			int newVecValue = 0;
			read(fd[i][0], &newVecValue, sizeof(int));
			printf("Argument num = %d, argument value = %d\n", i, newVecValue);
        }
    }
    return 0;    
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
