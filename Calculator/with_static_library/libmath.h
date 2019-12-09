#include <stdio.h>

#ifndef LIBMATH_H
#define LIBMATH_H

struct complexNum{
  double re;
  double im;
};

void enter_complex_numbers();
void addition();
void subtraction();
void multiplication();
int division();
void print_result();

#endif /* LIBMATH_H */