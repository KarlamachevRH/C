#--- Makefile for LW10
LW10: LW10.o
	gcc -o LW10 LW10.o -L. -lpthread

LW10.o: LW10.c
	gcc -std=c99 -Wall -g3 -c LW10.c

clean:
	rm -f *.o LW10
#---------------
