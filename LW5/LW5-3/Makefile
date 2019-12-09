#--- Makefile for arithmetic
arithmetic: main.o libarithmetic.so
	gcc -o arithmetic main.o -ldl

main.o: main.c
	gcc -std=c99 -Wall -c main.c

libarithmetic.so: arithmetic.o	
	gcc -shared -o libarithmetic.so arithmetic.o

arithmetic.o: arithmetic.c
	gcc -std=c99 -Wall -c -fPIC arithmetic.c

clean:
	rm -f *.o *.so arithmetic
#---------------
