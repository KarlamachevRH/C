/* 
 * For POSIX shared memory programs realization uncomment #define SHARED_MEMORY_POSIX 
 * and comment #define SHARED_MEMORY_SYSTEM_V in header.h file
 */

#include "headers.h"

int main(int argc, char **argv) 
{
	char str_for_send[MAX_BUF_SIZE] = {0};
	char str_for_receive[MAX_BUF_SIZE] = {0};
	
	log_info("Start of server program");
	snprintf(str_for_send, MAX_BUF_SIZE, "%s", "Hi, Dude!");


#ifdef SHARED_MEMORY_SYSTEM_V

	if((key = ftok(path_name, proj_id)) < 0)
	{
        perror("ftok");
        exit(EXIT_FAILURE);
    }
	/* 
	 * Создадим 2 семафора - для синхронизации записи/чтения с разделяемой памяти.
	 * Первый семафор для контроля записи в разделяемую память.
	 * Второй семафор контролирует последовательность запуска сервера и клиента
	 * (сервер, затем клиент), а также готовность сообщения от клиента (клиент оповещает
	 * о наличии сообщения от него)
	 */
	sem_init();

	/* Создадим 2 семафора - для синхронизации (записи/чтения) с разделяемой памятью.*/
    if((sem_id = semget(key, 2, 0660 | IPC_CREAT | IPC_EXCL)) == -1)
	{
		if((sem_id = semget(key, 2, O_RDWR)) == -1) /* If client started first */
		{
			perror("semget");
			exit(EXIT_FAILURE);
		}
	}

	/* Создадим буфер разделяемой памяти */
	if((shm_id = shmget(key, MAX_BUF_SIZE, IPC_CREAT | IPC_EXCL | 0660)) == -1)
	{
		if((shm_id = shmget(key, MAX_BUF_SIZE, O_RDWR)) == -1) /* If client started first */
		{
			perror("msgget");
			exit(EXIT_FAILURE);
		}
	}

	 /* Установить в семафорах №0 и №1 (Контроллеры ресурса) значение "1" */
	arg.val = 1;
	semctl(sem_id, 0, SETVAL, arg);
	semctl(sem_id, 1, SETVAL, arg);

	/* Получим доступ к разделяемой памяти */
	if (*(int*)(shm = (char*)shmat(shm_id, NULL, 0)) == -1)
	{
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	/* Запишем 0 в семафор 1 для оповещения клиента о запуске сервера */
	if((semop(sem_id, &client_sem_lock, 1)) == -1)
	{
		fprintf(stderr, "Semaphore decrement failed\n");
		exit(EXIT_FAILURE);
	}

	/* Заблокируем разделяемую память для записи сообщения клиенту */
	if((semop(sem_id, &serv_sem_lock, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}

	strncpy(shm, str_for_send, MAX_BUF_SIZE);

	/* Разблокируем разделяемую память */
	if((semop(sem_id, &serv_sem_unlock, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}
	
	/* Ожидаем записи клиента в разделяемую память */
	if((semop(sem_id, &client_sem_lock, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}

	strncpy(str_for_receive, shm, MAX_BUF_SIZE);
	printf("This is string from client: %s\n", str_for_receive);

	/* Разблокируем разделяемую память */
	if((semop(sem_id, &client_sem_unlock, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}

	if ((rc = shmctl(shm_id, IPC_RMID, NULL)) < 0)
	{
		perror("shmctl");
		log_info("shmctl (return shared mem) failed, rc=%d\n", rc);
		exit(EXIT_FAILURE);
	}

	if (semctl(sem_id, 0, IPC_RMID) < 0) 
	{
		printf("Невозможно удалить семафор\n");
		exit(EXIT_FAILURE);
	}

	log_info("End of server program");
	exit(EXIT_SUCCESS);

#endif // SHARED_MEMORY_SYSTEM_V


#ifdef SHARED_MEMORY_POSIX

	mode_t mode = 0666;
	char *addr;

	/* Создадим 2 семафора - для синхронизации (записи/чтения) с разделяемой памятью.*/
	if((sid1 = sem_open(sem_name1, O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
	{
		if((sid1 = sem_open(sem_name1, 0)) == SEM_FAILED) /* If client started first */
		{
			perror("sem_open1");
			exit(EXIT_FAILURE);
		}
	}
	if((sid2 = sem_open(sem_name2, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
	{
		if((sid2 = sem_open(sem_name2, 0)) == SEM_FAILED) /* If client started first */
		{
			perror("sem_open2");
			exit(EXIT_FAILURE);
		}
	}
	/* Создадим 3 семафор - для контроля последовательности запуска сервера и клиента */
	if((sid3 = sem_open(sem_name3, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
	{
		if((sid3 = sem_open(sem_name3, 0)) == SEM_FAILED) /* If client started first */
		{
			perror("sem_open2");
			exit(EXIT_FAILURE);
		}
	}

	if((shm_id = shm_open(shm_name, O_CREAT | O_RDWR | O_EXCL, mode)) == -1)
	{
		if((shm_id = shm_open(shm_name, O_RDWR, 0)) == -1) /* If client started first */
		{
			perror("shm_open");
			exit(EXIT_FAILURE);
		}
		if (ftruncate(shm_id, MAX_BUF_SIZE) == -1)
		{
			perror("ftruncate");
			exit(EXIT_FAILURE);
		}
	}
	else 
	if(ftruncate(shm_id, MAX_BUF_SIZE) == -1)
	{
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	if((addr = mmap(NULL, MAX_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED)
	{
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	/* Запишем 1 в семафор 3 для оповещения клиента о запуске сервера */
	if(sem_post(sid3) == -1)
	{
		perror("sem_post3");
		exit(EXIT_FAILURE);
	}

	/* Заблокируем разделяемую память для записи сообщения клиенту */
	if(sem_wait(sid1) == -1)
	{
		perror("sem_wait1");
		exit(EXIT_FAILURE);
	}

	strncpy(addr, str_for_send, MAX_BUF_SIZE);

	/* Разблокируем разделяемую память */
	if(sem_post(sid1) == -1)
	{
		perror("sem_post1");
		exit(EXIT_FAILURE);
	}

	/* Ожидаем записи клиента в разделяемую память */
	if(sem_wait(sid2) == -1)
	{
		perror("sem_wait2");
		exit(EXIT_FAILURE);
	}

	strncpy(str_for_receive, addr, MAX_BUF_SIZE);
	printf("This is string from client: %s\n", str_for_receive);

	/* Разблокируем разделяемую память */
	if(sem_post(sid2) == -1)
	{
		perror("sem_post2");
		exit(EXIT_FAILURE);
	}

	if(sem_close(sid1) == -1)
	{
		perror("sem_close1");
		exit(EXIT_FAILURE);
	}
	if(sem_close(sid2) == -1)
	{
		perror("sem_close2");
		exit(EXIT_FAILURE);
	}
	if(sem_close(sid3) == -1)
	{
		perror("sem_close3");
		exit(EXIT_FAILURE);
	}

	if(sem_unlink(sem_name1) == -1)
	{
		perror("sem_unlink");
		exit(EXIT_FAILURE);
	}

	if(sem_unlink(sem_name2) == -1)
	{
		perror("sem_unlink");
		exit(EXIT_FAILURE);
	}
	
	if(sem_unlink(sem_name3) == -1)
	{
		perror("sem_unlink");
		exit(EXIT_FAILURE);
	}

	if(close(shm_id) == -1)
	{
		perror("close shared mem fd");
		exit(EXIT_FAILURE);
	}

	if(shm_unlink(shm_name) == -1)
	{
		perror("shm_unlink");
		exit(EXIT_FAILURE);
	}
	log_info("End of server program");
	exit(EXIT_SUCCESS);

#endif // SHARED_MEMORY_POSIX
}