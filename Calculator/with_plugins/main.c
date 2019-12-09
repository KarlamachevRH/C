#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>

#define N 2
#define MAX_LIB_NUM 50
#define MAX_STR_SIZE 128

double a[N], b[N], result[N];
const char *funcName = "get_name";

void set_zero(double *x)
{
    int i;
    for (i = 0; i < N; i++)    
        x[i] = 0;  
}

void set_globals_to_zero()
{
    set_zero(a);
    set_zero(b);
    set_zero(result);
}

/* Открываем совместно используемую библиотеку */
void get_dl_handle(void *dl_handle, char *lib)
{    
    dl_handle = dlopen(lib, RTLD_LAZY|RTLD_GLOBAL);
    if (!dl_handle) 
    {
        printf("dlopen: %s\n", dlerror());        
        exit(EXIT_FAILURE);
    }
    (void) dlerror(); /* Clear dlerror() */
}

/* Get all libaries within directory "./plugin" */
int get_libraries_list(char **libNames)
{
    DIR *dir;    
    errno = 0;
    int i = 0;    
    struct dirent *entry;       
    if ((dir = opendir ("./plugin")) != NULL)
    {    
        while ((entry = readdir(dir)) != NULL && i < MAX_LIB_NUM)
        {            
            if ((strstr(entry->d_name, ".so")) != NULL)
            {                                             
                strcpy(libNames[i], entry->d_name); 
                i++;     
                //printf("libName #%d: %s\n", i, libNames[i]);                  
            }                                    
        }            
        closedir (dir);
    } 
    else 
    {
        /* could not open directory */
        perror ("");        
    }
    return errno;
}

/* Находим  имя функции в библиотеке */
void get_func_name(void *dl_handle, char *name)
{
    const char *err;
    char* (*fp)() = NULL;

    fp = dlsym(dl_handle, funcName); 
    err = dlerror();
    if (err != NULL)         
    {
        printf("dlsym: %s\n", err);        
        exit(EXIT_FAILURE);
    }    
    strcpy(name, fp());    
}

/* Вывести результат */
void print_result()
{
    if(result[1] > 0)
        printf("Result of operation: %.4lf+%.4lf*i\n", result[0], result[1]);
    else
        printf("Result of operation: %.4lf%.4lf*i\n", result[0], result[1]);
}

/* Находим адрес функции в библиотеке */
void get_func_address(void *dl_handle, char *name, int (*func)(double *a, double *b, double *result))
{
    const char *err;
    int error = 0;
    func = dlsym(dl_handle, name); 
    err = dlerror();
    if (err != NULL)         
    {
        printf("dlsym: %s\n", err);        
        exit(EXIT_FAILURE);
    }
    error = func(a, b, result);   
    if(!error)
        print_result(); 
}

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

/*  Ввод значений комплексных чисел для произведения расчетов */
void enter_complex_numbers(double *a, double *b)
{    
    printf("Enter first complex number:\n");
    printf("Enter real part:\n");
    scanf("%lf", a);
    printf("Enter imaginary part:\n");
    scanf("%lf", ++a);

    printf("Enter second complex number:\n");
    printf("Enter real part:\n");
    scanf("%lf", b);
    printf("Enter imaginary part:\n");
    scanf("%lf", ++b);    
}

/* Меню выбора доступных операций */
int menu(char **libNames)
{
    int op = -1;
    int i, operationNum = 0;
    char libName[MAX_STR_SIZE] = {0};
    int j = strlen("lib"); // индекс начала строки для вывода имени операции

    printf("Enter mathematic operation:\n");
    printf("No operation and exit program: %d\n", operationNum++);

    for(i = 0; libNames[i][0] && i < MAX_LIB_NUM; i++)
    {                          
        strncpy(libName, &libNames[i][j], strlen(&libNames[i][j]) - strlen(".so")); 
        printf("%s: %d\n", libName, operationNum++);
        memset(libName, 0, sizeof(char)*MAX_STR_SIZE);
    }       
    while(op < 0 || op > operationNum)
    {
        scanf("%d", &op);
        if(op < 0 || op > operationNum)
            printf("Wrong operation number, repeat entering number of operation:\n");
    }
    return op;
}

/* Подключение библиотеки и выполнение выбранной операции с заданными числами */
int calculate(char **libNames, void *dl_handle, 
              int (*func)(double *a, double *b, double *result))
{  
    int op = 0;    
    int isNextOperationEnabled = 1;
    char name[MAX_STR_SIZE] = {0};
    
    set_globals_to_zero();    
    enter_complex_numbers(a, b);

    errno = get_libraries_list(libNames);
    if(errno)
        printf("Getting libraries list error: %d\n", errno);        
    
    if(op = menu(libNames))
    {
        op--;
        get_dl_handle(dl_handle, libNames[op]);
        get_func_name(dl_handle, name);
        get_func_address(dl_handle, name, func);
    }
    else
    {
        printf("Exit program...\n");
        dlclose(dl_handle);
        isNextOperationEnabled = 0;
    } 
    (void) dlerror(); 
    return isNextOperationEnabled;
}

int main (int argc, char **argv)
{   
    int isNextOperationEnabled = 1;    
    int i;     
    void *dl_handle = NULL;
    
    char *libNames[MAX_LIB_NUM]; 

    for (i = 0; i < MAX_LIB_NUM; i++)
        libNames[i] = calloc(MAX_STR_SIZE, sizeof(char));       
    
    int (*func)(double *a, double *b, double *result) = NULL;

    printf("Calculator loaded\n");

    while(isNextOperationEnabled)
    {
        isNextOperationEnabled = calculate(libNames, dl_handle, func);
        isNextOperationEnabled = ask_for_exit();   
        for(i = 0; i < MAX_LIB_NUM; i++)         
            memset(libNames[i], 0, sizeof(char)*MAX_STR_SIZE);
    }    
    if(dl_handle != NULL)
        dlclose(dl_handle); 
    for (i = 0; libNames[i] != NULL && i < MAX_LIB_NUM; i++)
    {
        if(libNames[i] != NULL)
            free(libNames[i]);        
    }
    printf("Calculator unloaded\n");
    return EXIT_SUCCESS;
}
