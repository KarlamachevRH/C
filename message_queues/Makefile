#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=-lrt
SOURCES=server.c client.c headers.c
EXECUTABLE=server client

.PHONY: all clean

all: $(OBJECTS) $(EXECUTABLE)

server: headers.o server.o
	$(CC) headers.o server.o -o $@ $(LDFLAGS)

client: headers.o client.o
	$(CC) headers.o client.o -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)
	
#---------------