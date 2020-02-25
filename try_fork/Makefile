#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=
SOURCES=fork1.c fork5.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fork1 fork5

.PHONY: all clean

all: $(OBJECTS) $(EXECUTABLE)

$@: $<
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
#---------------