#include <stdio.h>

int main(void)
{
    struct test
    {
        char a;
        int b;
    }__attribute__((packed));

    char str [10] = {'A',0,0,0,0,'B',1,0,0,0};

    struct test *ptr;
    ptr = (struct test*)str;
    printf("char a = %c\nint b = %d\n", ptr->a, ptr->b);

}