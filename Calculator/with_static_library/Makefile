#--- Makefile for calculator
calculator: main.o libmath.a
	gcc -o calculator main.o -L. -lmath

main.o: main.c
	gcc -std=c99 -Wall -c -g main.c

libmath.a: libmath.o	
	ar rc libmath.a libmath.o
	ranlib libmath.a

libmath.o: libmath.c
	gcc -std=c99 -Wall -c -g libmath.c

clean:
	rm -f *.o *.a calculator
#---------------
