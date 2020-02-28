#define _XOPEN_SOURCE
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid, ppid;
    ppid = getppid();
    pid = getpid();
    printf("CHILD process executed. pid=%d, ppid=%d\n", pid, ppid);
    return 0;
}
