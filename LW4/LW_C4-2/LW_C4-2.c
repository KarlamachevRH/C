/*ЗАДАНИЕ:
В лабораторной работе требуется написать две программы для обработки текстовых файлов.Одна из них выполняет построчную, другая посимвольную обработку :
1. Написать программу, обрабатывающую текстовый файл и записывающую обработанные данные в файл с таким же именем, но с другим типом(табл. 3).
2. Написать программу, выполняющую посимвольную обработку текстового файла(табл. 4).

Ввод параметров должен быть организован в командной строке запуска программы.
Исходный файл должен быть создан с помощью любого текстового редактора.
При обработке текста рекомендуется использовать функции из стандартной библиотеки СИ для работы со строками, преобразования и анализа символов.

Таблица 3.
Вариант:
10
Исключить строки, содержащие заданное слово
Параметры командной строки:
1. Имя входного файла
2. Заданное слово

Таблица 4.
Вариант:
10
Заменить все пробелы первым символом текста
Параметры командной строки:
1. Имя входного файла
2. Количество замен */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FNAME_MAX 60

void write_new_file(char *exchangesCnt, FILE *fpRead, FILE *fpWrite) {

	int exchanges = atoi(exchangesCnt);
	char firstLetter = fgetc(fpRead);	
	printf("%d\n", firstLetter);
	fputc(firstLetter, fpWrite);	
	char ch = 0;
	int counter = 0;
	while (!feof(fpRead)) {
		ch = fgetc(fpRead);
		if (ch == ' ' && counter < exchanges) {
			fputc(firstLetter, fpWrite);
			counter++;
		}
		else fputc(ch, fpWrite);
	}
}

void output_file_name(char *argv, char *fileName) { //получение имени файла

	int i = 0;
	const char *extension = "2nd";
	for (; argv[i] != '.' && argv[i] != '\0'; i++) {
		fileName[i] = argv[i];
	}
	fileName[i] = '.'; i++;
	for (int j = 0; extension[j] != '\0'; j++, i++) {
		fileName[i] = extension[j];
	}
	fileName[i] = '\0';
}

int main(int argc, char *argv[]) {	
	FILE *fpRead, *fpWrite;	
	char fileName[FNAME_MAX];

	if ((fpRead = fopen(argv[1], "r")) == NULL) {
		printf("Невозможно открыть файл!\n");
		exit(1);
	}

	output_file_name(argv[1], fileName);

	if ((fpWrite = fopen(fileName, "a")) == NULL) {
		printf("Невозможно открыть файл!\n");
		exit(1);
	}

	if (!argv[2]) printf("Введите количество замен!\n");

	write_new_file(argv[2], fpRead, fpWrite);

	fclose(fpRead);
	fclose(fpWrite);
	return 0;
}
