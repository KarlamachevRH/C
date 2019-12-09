#include "libmath.h"

struct complexNum a, b, result;

void enter_complex_numbers()
{    
    a.re = 0;
    b.im = 0;
    printf("Enter first complex number:\n");
    printf("Enter real part:\n");
    scanf("%lf", &a.re);
    printf("Enter imaginary part:\n");
    scanf("%lf", &a.im);

    printf("Enter second complex number:\n");
    printf("Enter real part:\n");
    scanf("%lf", &b.re);
    printf("Enter imaginary part:\n");
    scanf("%lf", &b.im);    
}

void addition()
{
    result.re = 0;
    result.im = 0;
    result.re = a.re + b.re;
    result.im = a.im + b.im;    
}

void subtraction()
{
    result.re = 0;
    result.im = 0;
    result.re = a.re - b.re;
    result.im = a.im - b.im;    
}

void multiplication()
{
    result.re = 0;
    result.im = 0;
    result.re = a.re * b.re - a.im * b.im;
    result.im = a.im * b.re + a.re * b.im;    
}

int division()
{
    result.re = 0;
    result.im = 0;  
    int err = 0;
    if(b.re > 0 || b.im > 0 || b.re < 0 || b.im < 0)
    {
        result.re = (a.re * b.re + a.im * b.im)/(b.re * b.re + b.im * b.im);
        result.im = (a.im * b.re - a.re * b.im)/(b.re * b.re + b.im * b.im);
    }
    else    
    {
        printf("Error: divison by zero!\n");
        err = 1;
    }
    return err;           
}

void print_result()
{
    if(result.im > 0)
        printf("Result of operation: %.4lf+%.4lf*i\n", result.re, result.im);
    else
        printf("Result of operation: %.4lf%.4lf*i\n", result.re, result.im);
}