#--- Makefile for LW10
server: server.o client1 client2
	gcc -o server server.o amessage.pb-c.c -lprotobuf-c

server.o: server.c
	gcc -std=c99 -Wall -g3 -c server.c

client1: client1.o 
	gcc -o client1 client1.o amessage.pb-c.c -lprotobuf-c

client1.o: client1.c
	gcc -std=c99 -Wall -g3 -c client1.c 

client2: client2.o 
	gcc -o client2 client2.o amessage.pb-c.c -lprotobuf-c

client2.o: client2.c
	gcc -std=c99 -Wall -g3 -c client2.c

clean:
	rm -f *.o server client1  client2
#---------------
