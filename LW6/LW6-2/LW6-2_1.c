#include <stdio.h>
#include <stdlib.h>

#define N 3

int matrix_str_multiplication(int *matrixStr, int *vector); //Вычисление аргумента нового вектора
void read_data(int *matrix, int *vector, char **argv); //Чтение данных из файла в массивы матрицы и вектора

int main(int argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <matrix string> <vector for multiplication>\n", argv[0]);
		return(-1);
	}

    int matrixStr[N];         
    int vector[N];
    read_data(matrixStr, vector, argv);
    int newVecValue = -1;    
    newVecValue = matrix_str_multiplication(matrixStr, vector);    
    return newVecValue;   
}

void read_data(int *matrixStr, int *vector, char **argv){
    int data = 0;
    int i = 0, j = 0;

    FILE *ptrMatrix;
    ptrMatrix = fopen(argv[1], "r");
    if (ptrMatrix == NULL) {
        printf("Ошибка открытия файла данных матрицы!\n");
        exit(-1);        
    }
    i = 0;
    while(fscanf (ptrMatrix, "%d", &data) != EOF && i < N) {
        for(j = 0; j < N; j++){
            matrixStr[i] = data;
        }
        i++; 
    }
    fclose(ptrMatrix);

    FILE *ptrVector;
    ptrVector = fopen(argv[2], "r");
    if (ptrVector == NULL) {
        printf("Ошибка открытия файла вектора!\n");
        exit(-1);        
    }
    i = 0;
    while(fscanf (ptrMatrix, "%d", &data) != EOF && i < N) {        
        vector[i] = data;        
        i++; 
    }           
    fclose(ptrVector);
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