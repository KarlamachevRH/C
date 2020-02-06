#include <stdio.h>

char NAME[] = "multiplication";

int multiplication(double *a, double *b, double *result)
{
    // real part    
    result[0] = a[0] * b[0] - a[1] * b[1];

    // imaginary part
    result[1] = a[1] * b[0] + a[0] * b[1]; 
    return 0; 
}