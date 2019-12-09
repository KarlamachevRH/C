/*ЗАДАНИЕ:
1.
Написать программу, работающую с базой данных в виде массива структур и выполняющую последовательный ввод данных в массив и последующую распечатку его содержимого.
Состав структуры приведен в варианте.
Типы данных выбрать самостоятельно.
При написании программы следует использовать статические массивы структур или указателей на структуру.
Размерности массивов – 3–4.
Ввод данных выполнить с помощью функций scanf().

2.
Модифицировать программу, используя массив указателей на структуру и динамическое выделение памяти.
Выполнить сортировку массива с помощью функции qsort.
Способ сортировки массива приведен в варианте.
Для динамического выделения памяти используйте функцию malloc().
Для определения размера структуры в байтах удобно использовать операцию sizeof(), возвращающую целую константу :
struct ELEM *sp;
sp = malloc(sizeof(struct ELEM));

Вариант:

7
Фамилия
Группа
Номер в списке
Стипендия
Расположить записи в порядке возрастания номера в списке*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NAMEMAX 50

typedef struct STUDENTS {
	char secondName[NAMEMAX];
	char group[NAMEMAX];
	int listNum;
	int stipend;
} STUDENTS;

void enter_data(STUDENTS **student, int N); //ввод данных
void print_data(STUDENTS **student, int N); //вывод данных
int struct_elements_compare(const void * p1, const void * p2); //сравнение элементов массива структур для сортировки

int main(){	
	const int N = 3;

	STUDENTS **student = (STUDENTS**)malloc(N * sizeof(STUDENTS*));
	
	enter_data(student, N);
	qsort(&student[0], N, sizeof(STUDENTS*), struct_elements_compare);
	print_data(student, N);

	for (int i = 0; i < N; i++) {
		if (student[i] != NULL) free(student[i]);				
	}
	free(student);
	return 0;
}

void enter_data(STUDENTS **student, int N) {

	for (int i = 0; i < N; i++) {

		student[i] = (STUDENTS*)malloc(sizeof(struct STUDENTS));

		printf("Enter student's list number: ");
		scanf("%d", &student[i]->listNum);

		printf("Enter student's second name: ");
		scanf("%s", student[i]->secondName);

		printf("Enter student's group: ");
		scanf("%s", student[i]->group);

		printf("Enter student's stipend: ");
		scanf("%d", &student[i]->stipend);
	}
}

void print_data(STUDENTS **student, int N) {

	printf("Num | Second name     | Group     | Stipend\n");
	printf("----+-----------------+-----------+--------\n");

	for (int i = 0; i < N; i++) {

		printf("%3d | %-15s | %-9s | %-7d\n", student[i]->listNum, student[i]->secondName, student[i]->group, student[i]->stipend);
		
	}
	printf("----+-----------------+-----------+--------\n");
}

int struct_elements_compare(const void * p1, const void * p2) {

	STUDENTS *student1 = *(STUDENTS**)p1;
	STUDENTS *student2 = *(STUDENTS**)p2;	
	int res = 0;
	if (student1->listNum > student2->listNum) res = 1;
	else if (student1->listNum < student2->listNum) res = -1;
	return res;
}
