#include <stdio.h>
#include <stdlib.h>


enum operation
{
    NOOPERATION,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION
};

int ask_for_exit()
{
    int choice = -1;
    while(choice < 0 || choice > 1)
    {
        printf("Enter 0 for exit programm or 1 for calculate:\n");
        scanf("%d", &choice);
    }
    return choice;
}

int menu()
{
    int op = -1;
    printf("Enter mathematic operation:\n");
    printf("No operation and exit program.............0\n");
    printf("Addition:.................................1\n");
    printf("Subtraction:..............................2\n");
    printf("Multiplication:...........................3\n");
    printf("Division:.................................4\n");
    while(op < NOOPERATION || op > DIVISION)
    {
        scanf("%d", &op);
        if(op < NOOPERATION || op > DIVISION)
            printf("Wrong operation number, repeat entering number of operation:\n");
    }
    return op;
}

int main (int argc, char **argv)
{
    int err;
    printf("Calculator loaded\n");
    while(1)   
    {                
        if (ask_for_exit())
        {
            err = 0;
            enter_complex_numbers();
            switch (menu())
            {
                case ADDITION:
                    addition();
                    break;
                
                case SUBTRACTION:
                    subtraction();
                    break;

                case MULTIPLICATION:
                    multiplication();
                    break;

                case DIVISION:
                    err = division();
                    break;

                case NOOPERATION:
                    exit(EXIT_SUCCESS);
                    break;

                default:
                    break;
            }
            if(!err)
                print_result();            
        }
        else 
            break;
    }
    return 0;
}
