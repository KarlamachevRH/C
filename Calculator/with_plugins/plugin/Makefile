#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 -fPIC
LDFLAGS=
SOURCES=libaddition.c libsubtraction.c libmultiplication.c libdivision.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=libaddition.so libsubtraction.so libmultiplication.so libdivision.so

.PHONY: all clean

all: $(SOURCES) $(EXECUTABLES)

%.so: %.o
	$(CC) -shared $< -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLES)

#---------------