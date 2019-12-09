/*ЗАДАНИЕ:
1.Разработать функции для выполнения арифметических операций по вариантам. 
2.Оформить статическую библиотеку функций и написать программу, ее использующую.
3.Переоформить библиотеку, как динамическую, но подгружать статически, при компиляции.
4.Изменить программу для динамической загрузки функций из библиотеки.

Вариант задания. 
2.Операции умножения и деления.*/

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <first number(digit)> <second number(digit)>\n", argv[0]);
		exit(-1);
	}
	double a = atof(argv[1]), b = atof(argv[2]);

	void *library_handler = NULL;
	double (*mult)(double, double);
	double (*div)(double, double);

	//загрузка библиотеки
	library_handler = dlopen("./libarithmetic.so",RTLD_LAZY);
	if (!library_handler){
		//если ошибка, то вывести ее на экран
		fprintf(stderr,"dlopen() error: %s\n", dlerror());
		exit(1); 
	};
	//получить адрес функции сложения
	mult = dlsym(library_handler, "multiplication");
	if (mult == NULL) {
		fprintf(stderr,"Can't find multiplication function: %s\n",dlerror());
		exit(-2);
 	}
	//получить адрес функции деления
	div = dlsym(library_handler, "division");
	if (div == NULL) {
		fprintf(stderr,"Can't find division function: %s\n",dlerror());
		exit(-2);
 	}

	double m = mult(a, b); //сложение
	double d = div(a, b);  //деление
	printf("Results of multiplication: %.2f, divison: %.2f\n", m, d);

	dlclose(library_handler);
	return 0;
}