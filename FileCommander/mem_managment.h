#ifndef __MEM_MANAGMENT_H__
#define __MEM_MANAGMENT_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>

typedef struct dirent direntry;

typedef direntry varType;

/* Get memory for two-dimensional array */
varType** mem_calloc(varType** namelist, int num);

/* Realloc memory for two-dimensional array */
varType** mem_realloc(varType** namelist, int numOld, int numNew);

/* Free memory from two-dimensional array */
void free_mem(varType** namelist, int num);

void set_zero_values(varType** namelist, int num);

#endif // __MEM_MANAGMENT_H__