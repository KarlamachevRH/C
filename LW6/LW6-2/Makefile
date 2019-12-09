#--- Makefile for LW6-1
LW6-2: LW6-2_2.o LW6-2_1
	gcc -o LW6-2 LW6-2_2.o

LW6-2_2.o: LW6-2_2.c
	gcc -std=c99 -Wall -c -g LW6-2_2.c

LW6-2_1: LW6-2_1.o 
	gcc -o LW6-2_1 LW6-2_1.o

LW6-2_1.o: LW6-2_1.c
	gcc -std=c99 -Wall -c -g LW6-2_1.c

clean:
	rm -f *.o LW6-2 LW6-2_1
#---------------
