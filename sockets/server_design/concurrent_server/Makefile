#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=-pthread
SOURCES=server.c client.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server client

.PHONY: all clean

all: $(EXECUTABLE)

server: server.o
	$(CC) $< -o $@

client: client.o
	$(CC) $< -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)
	
#---------------