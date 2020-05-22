#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=
SOURCES=server.c client.c get_dest_mac.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server client

.PHONY: all clean

all: $(OBJECTS) $(EXECUTABLE)

client: get_dest_mac.o

$@: $<
	$(CC) $< -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)
	
#---------------