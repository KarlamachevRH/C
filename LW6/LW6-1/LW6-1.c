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
    int pid, newVecValue = 0;       
        // запускаем дочерний процесс 
        pid = fork(); 
        if (-1 == pid) {
            perror("fork"); /* произошла ошибка */
            exit(1); /*выход из родительского процесса*/
        } else if (0 == pid) { 
            printf(" CHILD: Это процесс-потомок.\n");
            newVecValue = matrix_str_multiplication(matrix[1], vector); 
            printf("(PIO: %d), New vector argument = %d\n", getpid(), newVecValue);                     
            exit(newVecValue); /* выход из процесс-потомока */            
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
