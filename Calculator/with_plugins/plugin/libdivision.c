#include <stdio.h>

char *NAME = "division";

char* get_name()
{
    return NAME;
} 

int division(double *a, double *b, double *result)
{
    int err = 0;
     
    if(b[0] > 0 || b[1] > 0 || b[0] < 0 || b[1] < 0)
    {
        // real part 
        result[0] = (a[0] * b[0] + a[1] * b[1])/(b[0] * b[0] + b[1] * b[1]);
        // imaginary part
        result[1] = (a[1] * b[0] - a[0] * b[1])/(b[0] * b[0] + b[1] * b[1]);
    }
    else    
    {
        printf("Error: divison by zero!\n");
        err = 1;
    }
    return err;
}