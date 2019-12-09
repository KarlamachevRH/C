/*ЗАДАНИЕ:
1.Разработать функции для выполнения арифметических операций по вариантам. 
2.Оформить статическую библиотеку функций и написать программу, ее использующую.
3.Переоформить библиотеку, как динамическую, но подгружать статически, при компиляции.
4.Изменить программу для динамической загрузки функций из библиотеки.

Вариант задания. 
2.Операции умножения и деления.*/

#include "arithmetic.h"

int main(int argc, char *argv[]){
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <first number(digit)> <second number(digit)>\n", argv[0]);
		return(-1);
	}
	double a = atof(argv[1]), b = atof(argv[2]);
	double m = multiplication(a, b);
	double d = division(a, b);
	printf("Results of multiplication: %.2f, divison: %.2f\n", m, d);
	return 0;
}