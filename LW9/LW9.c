/*  МЕТОДИЧЕСКИЕ УКАЗАНИЯ. 
1. Для обмена данными между процессами рекомендуется использовать разделяемую память. Для блокировки одновременного доступа процессов к разделяемой памяти рекомендуется использовать семафоры System V.
2. Для ознакомления с функциями работы с семафорами и разделяемой памятью используйте инструкции man semget, man semop, man semctl и man shmget, man shmat, man shmdt, man msgctl.
3. Для отладки рекомендуется использовать отладчик gdb, информацию котором можно получить инструкцией man gdb..
4. Для отладки и запуска программ, протоколирования их результатов и сохранения на локальном компьютере см. методические указания к лабораторной работе №6.
    ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ. 
1. Модифицировать и отладить в соответствии с вариантом и выбранным средством коммуникации программу из лабораторной работы №6, реализующую порожденный процесс. При необходимости модифицировать ее в отдельную программу-сервер.
2. Модифицировать и отладить в соответствии с вариантом и выбранным средством коммуникации программу из лабораторной работы №6, реализующую родительский процесс. При необходимости модифицировать ее в отдельную программу-клиент.
    ВАРИАНТ ЗАДАНИЯ.
4. Warcraft. Заданное количество юнитов добывают золото равными порциями из одной шахты, задерживаясь в пути на случайное время, до ее истощения. Работа каждого юнита реализуется в порожденном процессе. */

#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>

union semun {
 int val;                  /* значение для SETVAL */
 struct semid_ds *buf;     /* буферы для  IPC_STAT, IPC_SET */
 unsigned short *array;    /* массивы для GETALL, SETALL */
                           /* часть, особенная для Linux: */
 struct seminfo *__buf;    /* буфер для IPC_INFO */
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
		fprintf(stderr, "Usage: %s <Gold size (digit)> <Units number (digit)>\n", argv[0]);
		exit(-1);
	}
    int goldSize = atoi(argv[1]), unitsNum = atoi(argv[2]);

	pid_t pid[unitsNum];
    pid_t wpid;
    int status = -1;

    int shmid;
    key_t key = 77;
    int *shm;

    int semid;
    union semun arg;
    struct sembuf lock_res = {0, -1, 0};  //блокировка ресурса
    struct sembuf rel_res = {0, 1, 0};	//освобождение ресурса

    /* Получим ключ. Один и тот же ключ можно использовать как
    для семафора, так и для разделяемой памяти */
    if((key = ftok(".", 'S')) < 0){
        printf("Невозможно получить ключ\n");
        exit(1);
    }

    /* Создадим семафор - для синхронизации работы с разделяемой памятью.*/
    semid = semget(key, 1, 0666 | IPC_CREAT);

    /* Установить в семафоре № 0 (Контроллер ресурса) значение "1" */
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);
        
    /* Создадим область разделяемой памяти */
    if((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* Получим доступ к разделяемой памяти */
    if ((shm = (int*)shmat(shmid, NULL, 0)) == (int*)-1) {
        perror("shmat");
        exit(1);
    }

    /* Запишем в разделяемую память  объем золота в шахте */
    *(shm) = goldSize;

    int goldDecrement = 5;           

    for (int i = 0; i < unitsNum; i++){		
        pid[i] = fork();
        srand(getpid());
        if (0 == pid[i]) {            
            while(1){                
                printf("PID=%d i=%d\n", getpid(), i);         
                /* Получим доступ к разделяемой памяти */
                if ((shm = (int*)shmat(shmid, NULL, 0)) == (int*)-1) {
                    perror("shmat");
                    printf("Ошибка доступа к разделяемой памяти\n");
                    exit(1);
                }

                /* Заблокируем разделяемую память */	
                if((semop(semid, &lock_res, 1)) == -1){
                    fprintf(stderr, "Lock failed\n");
                    exit(1);
                } else{
                    printf("Semaphore resources decremented by one (locked) i=%d\n", i);
                    fflush(stdout);
                }
                
                /* Запишем в разделяемую память обновленный объем золота */
                if(*(shm)/goldDecrement > 0) *(shm) = *(shm) - goldDecrement;
                goldSize = *(shm);

                /* Освободим разделяемую память */
                if((semop(semid, &rel_res, 1)) == -1){
                        fprintf(stderr, "Unlock failed\n");
                        exit(1);
                } else{
                    printf("Semaphore resources incremented by one (unlocked) i=%d\n", i);
                    fflush(stdout);
                }
                
                printf("Gold size [i=%d] = %d\n", i, *(shm));                
                fflush(stdout);
                
                /* Отключимся от разделяемой памяти */
                if (shmdt(shm) < 0) {
                    printf("Ошибка отключения\n");
                    exit(1);
                }
                printf("Процесс ожидает PID=%d i=%d\n", getpid(), i);
                fflush(stdout);
                sleep(rand() % 4);
                if(goldSize == 0) exit(0);                         
            }                                          
        }	else if (pid[i] < 0){
                perror("fork"); /* произошла ошибка */
                exit(1); /*выход из родительского процесса*/
        }		
    }	
	
	for (int i = 0; i < unitsNum; i++) {
		wpid = waitpid(pid[i], &status, 0);
		if (pid[i] == wpid) {
			printf("процесс-потомок %d завершен, результат = %d\n", i, WEXITSTATUS(status));
			fflush(stdout);                       
		}
	}	

	/* Получим доступ к разделяемой памяти */
	if ((shm = (int*)shmat(shmid, NULL, 0)) == (int*) -1) {
		perror("shmat");
		exit(1);
	}

	printf("------------------\n");
	printf("Gold size = %d\n", *(shm));
	fflush(stdout);

	if (shmdt(shm) < 0) {
		printf("Ошибка отключения\n");
		exit(1);
	} 
	
	/* Удалим созданные объекты IPC */	
	 if (shmctl(shmid, IPC_RMID, 0) < 0) {
		printf("Невозможно удалить область\n");
		exit(1);
	} else
		printf("Сегмент памяти помечен для удаления\n");
	
	if (semctl(semid, 0, IPC_RMID) < 0) {
		printf("Невозможно удалить семафор\n");
		exit(1);
	}	
    return 0;   
}