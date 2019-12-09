#include <stdio.h>

char *NAME = "subtraction";

char* get_name()
{
    return NAME;
} 

int subtraction(double *a, double *b, double *result)
{
    // real part    
    result[0] = a[0] - b[0];
    
    // imaginary part
    result[1] = a[1] - b[1];
    return 0; 
}