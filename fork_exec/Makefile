#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=
SOURCES=exec.c test.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=exec test

.PHONY: all clean

all: $(OBJECTS) $(EXECUTABLE)

$@: $<
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
#---------------