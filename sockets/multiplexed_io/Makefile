#--- Makefile

CC=gcc
CFLAGS=-c -g -Wall -std=c99 
LDFLAGS=
SERV_SOURCE=server.c
SOURCES=udp_client.c tcp_client.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=select_server poll_server epoll_server udp_client tcp_client

.PHONY: all clean

all: $(SOURCES) $(EXECUTABLES)

select_server: CFLAGS += -D SELECT

poll_server: CFLAGS += -D POLL

epoll_server: CFLAGS += -D EPOLL

$@: $<
	$(CC) $< -o $@ $(LDFLAGS)

%_server.o: $(SERV_SOURCE) headers.h
	$(CC) $(CFLAGS) $(SERV_SOURCE) -o $@

$(OBJECTS): headers.h

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLES)
	
#---------------