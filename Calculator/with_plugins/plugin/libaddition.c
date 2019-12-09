#include <stdio.h>

char *NAME = "addition";

char* get_name()
{
    return NAME;
} 

int addition(double *a, double *b, double *result)
{
    // real part    
    result[0] = a[0] + b[0];
    
    // imaginary part
    result[1] = a[1] + b[1];  
    return 0;
}