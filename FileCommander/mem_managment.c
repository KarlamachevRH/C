#include "mem_managment.h"

/* Get memory for two-dimensional array */
varType** mem_calloc(varType** namelist, int num)
{    
    if((namelist = (varType**)calloc(num, sizeof(varType*))) != NULL)
    {
        for (int i = 0; i < num; i++)
        {
            if(!(namelist[i] = calloc(1, sizeof(varType))))
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
    return namelist;
}

/* Realloc memory for two-dimensional array */
varType** mem_realloc(varType** namelist, int numOld, int numNew)
{
    if(numOld > 0)
        free_mem(namelist, numOld);    
    namelist = mem_calloc(namelist, numNew);
    return namelist;
} 

/* Free memory from two-dimensional array */
void free_mem(varType** namelist, int num)
{
    if(namelist != NULL)
    {
        for (int i = 0; i < num; i++)
        {
            if(namelist[i])        
                free(namelist[i]);
        }
        free(namelist);
        namelist = NULL;
    }
}

void set_zero_values(varType** namelist, int num)
{
    for(int i = 0; i < num; i++)    
        memset(namelist[i], 0, num*sizeof(varType));
}