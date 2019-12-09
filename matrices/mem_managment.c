#include "mem_managment.h"

/* Get memory for two-dimensional array */
int** mem_calloc(int** memptr, int num)
{
    if((memptr = (int**)calloc(num, sizeof(int*))) != NULL)
    {
        for (int i = 0; i < num; i++)
        {
            if(!(memptr[i] = calloc(num, sizeof(int))))
            {
                perror ("Error");
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        perror ("Error");
        exit(EXIT_FAILURE);
    }
    return memptr;
} 

/* Free memory from two-dimensional array */
void free_mem(int** memptr, int num)
{
    if(memptr)
    {
        for (int i = 0; i < num; i++)
        {
            if(memptr[i])        
                free(memptr[i]);        
        }   
        free(memptr);
    }    
}

void set_zero_values(int** memptr, int num)
{
    for(int i = 0; i < num; i++)    
        memset(memptr[i], 0, sizeof(int)*num);
}