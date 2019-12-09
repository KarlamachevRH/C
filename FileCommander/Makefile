#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=-lpthread -lpanel -lmenu -lncurses
SOURCES=main.c mem_managment.c cursed_window.c cursed_menu.c fork_proccess.c pthread_create.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fc

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
#---------------