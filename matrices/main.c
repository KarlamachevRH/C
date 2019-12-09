#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mem_managment.h"


void set_rand_num(int **matrix, int size)
{
    srand(time(NULL));      /* Initialization, should only be called once */ 
    for(int i = 0; i < size; i++)
    {   
        for(int j = 0; j < size; j++)     
            matrix[i][j] = rand() % 10; /* random int between 0 and 10 */               
    }

}

void print_matrix(int **matrix, int size)
{
    printf("---------------Matrix---------------\n"); 
    int centralNum = size/2;  
    for(int i = 0; i < size; i++)
    {   
        for(int j = 0; j < size; j++)
        {
            if(matrix[centralNum][centralNum] < 99)     
                printf("%3d", matrix[i][j]);
            else
                printf("%5d", matrix[i][j]);
        }
        
        printf("\n");        
    }
}

void set_triangles(int **matrix, int size)
{
    int i, j, k;

    set_zero_values(matrix, size);

    for(i = 0, k = 0; i < size; i++, k++)
    {
        j = k;           
        while(j < size) 
        {
            matrix[i][j] = 1;
            j++;
        }         
    }
}

void set_helix(int **matrix, int size)
{    
    int i = 0, j, k = 0, p = 1;
    int numCounter = size*size;

    set_zero_values(matrix, size);    

    while(i < numCounter)
    {
        k++;
        for (j = k-1; j < size-k+1; j++)
        {
            matrix[k-1][j] = p++;
            i++;
        }   
    
        for (j = k;j < size-k+1; j++)
        {
            matrix[j][size-k] = p++;
            i++;
        }   
    
        for (j = size-k-1; j >= k-1; j--)
        {
            matrix[size-k][j] = p++;
            i++;
        }   
    
        for (j = size-k-1; j >= k; j--)
        {
            matrix[j][k-1] = p++;
            i++;
        }   
    }
}

int main(int argc, char **argv)
{
    int **matrix = NULL;
    int size = 0;
    
    if(argc < 2)
    {
        printf("Usage: %s <quad matrix size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(!(size = atoi(argv[1])))
    {
        printf("Usage: %s size-integer\n", argv[0]);
        exit(EXIT_FAILURE);
    }
        
    matrix = mem_calloc(matrix, size);
    
    set_rand_num(matrix, size);

    print_matrix(matrix, size);

    set_triangles(matrix, size);

    print_matrix(matrix, size);

    set_helix(matrix, size);

    print_matrix(matrix, size);

    free_mem(matrix, size);

    return EXIT_SUCCESS;
}