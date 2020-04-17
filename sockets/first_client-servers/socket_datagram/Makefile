#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=
SOURCES=server.c client.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server client

.PHONY: all clean

all: $(OBJECTS) $(EXECUTABLE)

$@: $<
	$(CC) $< -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)
	
#---------------