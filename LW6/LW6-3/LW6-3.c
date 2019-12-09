/* ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ. 
1. Ознакомиться с опциями компилятора gcc, методикой отладки программ. 
2. Для вариантов заданий написать и отладить программу, реализующую порожденный процесс.
3. Для вариантов заданий написать и отладить программу, реализующую родительский процесс, вызывающий и отслеживающий состояние порожденных процессов - программ (ждущий их завершения или уничтожающий их, в зависимости от варианта).
4. Для вариантов заданий написать и отладить программу, реализующую родительский процесс, вызывающий и отслеживающий состояние порожденных процессов - функций (ждущий их завершения или уничтожающий их, в зависимости от варианта).

   ВАРИАНТ ЗАДАНИЯ.
2. Умножение матрицы на вектор. Обработка одной строки матрицы - в порожденном процессе.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

#define N 3
#define M 4

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

    for (i = 0; i < M; i++) {
        // запускаем дочерний процесс 
        pid[i] = fork();        
        if (-1 == pid[i]) {
            perror("fork"); /* произошла ошибка */
            exit(1); /*выход из родительского процесса*/
        } else if (0 == pid[i]) {
            printf(" CHILD: Это %d процесс-потомок!\n", i);           
            exit(matrix_str_multiplication(matrix[i], vector)); /* выход из процесс-потомока */
        }
    }
    sleep(1);
    // если выполняется родительский процесс
    printf("PARENT: Это процесс-родитель!\n");
    int resultVector[M] = {0, 0, 0, 0};
    // ожидание окончания выполнения всех запущенных процессов
    for (i = 0; i < M; i++) {
        status = waitpid(pid[i], &stat, WNOHANG);
        if (pid[i] == status) {
            //printf("Процесс-потомок %d завершен, result=%d\n", i, WEXITSTATUS(stat));
            resultVector[i] = WEXITSTATUS(stat);
        }
    }
    printf("Значения аргументов вектора - результата умножения:\n");
    for (i = 0; i < M; i++) {
        printf("%d\n", resultVector[i]);               
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