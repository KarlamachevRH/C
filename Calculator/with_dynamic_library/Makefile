#--- Makefile for calculator
calculator: main.o libmath.so
	gcc -o calculator main.o -Wl,-rpath,. -lmath
	
main.o: main.c
	gcc -std=c99 -Wall -c -g main.c

libmath.so: libmath.o
	gcc -shared -o libmath.so libmath.o
	
libmath.o: libmath.c
	gcc -std=c99 -Wall -fPIC -c -g libmath.c

clean:
	rm -f *.o *.a *so calculator
#---------------
