#--- Makefile for LW11
server: server.o client
	gcc -o server server.o 

server.o: server.c
	gcc -std=c99 -Wall -g3 -c server.c

client: client.o 
	gcc -o client client.o 

client.o: client.c
	gcc -std=c99 -Wall -g3 -c client.c

clean:
	rm -f *.o server client
#---------------
