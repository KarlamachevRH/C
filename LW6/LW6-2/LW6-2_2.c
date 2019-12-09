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
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

#define N 3
#define M 4

void write_matrix_string_to_file(int *matrixStr);
void write_vector_to_file(int *vector);

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
    write_vector_to_file(vector);
    int i, pid[M], status, stat; 

    for (i = 0; i < M; i++) { 
        write_matrix_string_to_file(matrix[i]);       
        // запускаем дочерний процесс 
        pid[i] = fork();        
        if (-1 == pid[i]) {
            perror("fork"); /* произошла ошибка */
            exit(-2); /*выход из родительского процесса*/
        } else if (0 == pid[i]) {             
            if (execl("./LW6-2_1","LW6-2_1", "matrix.txt", "vector.txt", NULL)<0) {
                printf("ERROR while start process\n");
                exit(-3);
		    }
            else printf("CHILD: Это %d процесс-потомок (pid=%d)!\n", i, pid[i]);  /* выход из процесс-потомока */                          
        } 
        sleep(1);       
    }    
    // если выполняется родительский процесс
    printf("PARENT: Это процесс-родитель!\n");
    int resultVector[M] = {0, 0, 0, 0};
    // ожидание окончания выполнения всех запущенных процессов
    for (i = 0; i < M; i++) {
        status = waitpid(pid[i], &stat, WNOHANG);
        if (pid[i] == status) {
            printf("Процесс-потомок %d завершен, result = %d\n", i, WEXITSTATUS(stat));
            resultVector[i] = WEXITSTATUS(stat);
        }
        else {
			 perror("PARENT: потомок не завершился успешно");
		}   
    }
    printf("Значения аргументов вектора - результата умножения:\n");
    for (i = 0; i < M; i++) {
        printf("%d\n", resultVector[i]);               
    }    
    return 0;   
}

void write_matrix_string_to_file(int *matrixStr){
    FILE *ptrMatrix;
    ptrMatrix = fopen("matrix.txt", "w");
    if (ptrMatrix == NULL) {
        printf("Ошибка создания файла matrix.txt!");
        exit(-1);        
    }    
    for(int i = 0; i < N; i++){
        fprintf(ptrMatrix, "%d ", matrixStr[i]);
    }       
    fclose(ptrMatrix);
}

void write_vector_to_file(int *vector){
    FILE *ptrVector;
    ptrVector = fopen("vector.txt", "w");
    if (ptrVector == NULL) {
        printf("Ошибка создания файла vector.txt!");
        exit(-1);        
    }    
    for(int i = 0; i < N; i++){
        fprintf(ptrVector, "%d ", vector[i]);
    }        
    fclose(ptrVector); 
}