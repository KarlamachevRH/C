#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef __MEM_MANAGMENT_H__
#define __MEM_MANAGMENT_H__

/* Get memory for two-dimensional array */
int** mem_calloc(int** memptr, int num);

/* Free memory from two-dimensional array */
void free_mem(int** memptr, int num);

void set_zero_values(int** memptr, int num);

#endif // __MEM_MANAGMENT_H__