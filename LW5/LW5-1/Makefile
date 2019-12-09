#--- Makefile for arithmetic
arithmetic: main.o libarithmetic.a
	gcc -o arithmetic main.o -L. -larithmetic

main.o: main.c
	gcc -std=c99 -Wall -c main.c

libarithmetic.a: arithmetic.o
	ar cr libarithmetic.a arithmetic.o

arithmetic.o: arithmetic.c
	gcc -std=c99 -Wall -c arithmetic.c

clean:
	rm -f *.o *.a arithmetic
#---------------
