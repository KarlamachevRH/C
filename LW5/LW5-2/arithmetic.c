#include "arithmetic.h"

double multiplication(double a, double b){
	return a*b;
}

double division(double a, double b){	
	if(b==0) {
		printf ("Error: Division by zero!");
		getchar();
		exit(EXIT_FAILURE);
	}
	return a/b;
}