#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=-lpthread
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pthread_create

.PHONY: all clean

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
#---------------