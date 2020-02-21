#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=-lpanel -lmenu -lncurses
SOURCES=main.c cursed_window.c cursed_menu.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=editor

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
#---------------