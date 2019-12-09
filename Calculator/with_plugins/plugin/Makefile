#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 -fPIC
LDFLAGS=
SOURCES=libaddition.c libsubtraction.c libmultiplication.c libdivision.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=libaddition.so libsubtraction.so libmultiplication.so libdivision.so

all: $(SOURCES) $(EXECUTABLES)

libaddition.so: libaddition.o 
	$(CC) -shared -o libaddition.so libaddition.o

libsubtraction.so: libsubtraction.o 
	$(CC) -shared -o libsubtraction.so libsubtraction.o

libmultiplication.so: libmultiplication.o 
	$(CC) -shared -o libmultiplication.so libmultiplication.o

libdivision.so: libdivision.o 
	$(CC) -shared -o libdivision.so libdivision.o

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLES)

#---------------
