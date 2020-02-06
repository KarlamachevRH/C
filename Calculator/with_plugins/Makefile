#--- Makefile

CC=gcc
DIR_ROOT = $(shell pwd)
DIR_PLUGINS = $(DIR_ROOT)/plugin
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=-ldl -Wl,-Bsymbolic,-rpath,$(DIR_PLUGINS)
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=calculator

.PHONY: all plugins clean

all: $(SOURCES) $(EXECUTABLE) plugins

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -rdynamic -o $@ $(LDFLAGS)

plugins:
	make -C $(DIR_PLUGINS);

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	make clean -C $(DIR_PLUGINS);
	
#---------------
