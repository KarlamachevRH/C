#--- Makefile for LW15 netlink user space executable
netlink_us_exec: netlink_us_exec.o
	gcc -o netlink_us_exec netlink_us_exec.o 

netlink_us_exec.o: netlink_us_exec.c
	gcc -std=c99 -Wall -g3 -c netlink_us_exec.c

clean:
	rm -f *.o netlink_us_exec
#---------------
