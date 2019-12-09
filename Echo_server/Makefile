#--- Makefile for echo server ------------------------------
echoRecv: echoRecv.o echoSend
	gcc -o echoRecv echoRecv.o 

echoRecv.o: echoRecv.c
	gcc -std=c99 -Wall -g3 -c echoRecv.c

echoSend: echoSend.o 
	gcc -o echoSend echoSend.o 

echoSend.o: echoSend.c
	gcc -std=c99 -Wall -g3 -c echoSend.c 

clean:
	rm -f *.o echoRecv echoSend
#-----------------------------------------------------------
