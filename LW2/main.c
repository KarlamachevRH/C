/*Написать программу сортировки массива строк по вариантам.
Строки вводить с клавиатуры. Сначала пользователь вводит кол-во строк потом сами строки.
Для тестирования использовать перенаправление ввода-вывода ($./myprog < test.txt).
Память для строк выделять динамически с помощью функций malloc или calloc.
Ввод данных, сортировку и вывод результатов оформить в виде функций.
Сортировку делать с помощью qsort если возможно, если нет то писать свой вариант сортировки.
Входные и выходные параметры функции сортировки указаны в варианте.

3 Вариант
Расположить строки в алфавитном порядке
Входные параметры:
1. Массив
2. Размерность массива
Выходные параметры:
1. Количество перестановок
2. Первая буква первой строки */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** readMas(char **str, int count) {	
	char **mas;  //указатель на массив указателей на строки
	mas = (char **)malloc(sizeof(char *)*count);// выделяем память для массива указателей
	for (int i = 0; i < count; i++) {		
		mas[i] = (char *)malloc((sizeof(char)*strlen(str[i + 2]))+1); //выделяем память для строки
		strcpy(mas[i], str[i+2]); //копируем строку в массив указателей
	}
	return mas;
}

void printMas(char **mas, int count) {
	for (int i = 0; i < count; i++) {
		printf("%s\n", mas[i]);
	}
}

int cmpstringp(const void *p1, const void *p2) {
	char *str1 = *(char **)p1;
	char *str2 = *(char **)p2;
	return strcmp(str1, str2);
}

int cnt_exchanges(char **mas, char **argv, int strNum) {
	int k=0, cnt = 0;
	for (int i = 0, j = 2; i<strNum; i++, j++) {
		if (mas[i][k] != argv[j][k]) cnt++;
	}
	return cnt;
}

void freeMas(char **mas, int count) {
	for (int i = 0; i < count; i++) {
		if(mas[i]!=NULL)free(mas[i]); // освобождаем память для отдельной строки
	}
	free(mas); // освобождаем памать для массива указателей на строки
}

int main(int argc, char** argv) {
			
	int cnt = 0;	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <amount of strings(digit)> <string1>...\n", argv[0]);
		return(-1);
	}
        int strNum = atoi(argv[1]);
        if (strNum == 0 || strNum < argc-2 || strNum > argc - 2) {
		fprintf(stderr, "Write right number of strings. Usage: %s <amount of strings(digit)> <string1>...\n", argv[0]);
		return(-1);
	}
        
	char **mas = NULL;
	int count = strNum;
	mas = readMas(argv, count);
	qsort(mas, count, sizeof(char*), cmpstringp);
	printMas(mas, count);
	cnt = cnt_exchanges(mas, argv, strNum)/2;

	printf("First letter of first line: %c\n", mas[0][0]);

	printf("Counter of lines exchanges: %d\n", cnt);

	freeMas(mas, count);
	return(0);
}
