#include "headers.h"

int main(int argc, char **argv) 
{
	char str_for_send[MAX_BUF_SIZE] = {0};
	char str_for_receive[MAX_BUF_SIZE] = {0};
	
	log_info("Start of client program");
	snprintf(str_for_send, MAX_BUF_SIZE, "%s", "Hello!");


#ifdef SHARED_MEMORY_SYSTEM_V

	if((key = ftok(path_name, proj_id)) < 0)
	{
        perror("ftok");
        exit(EXIT_FAILURE);
    }	
	sem_init();

	/* Получим доступ к 2 семафорам - для синхронизации записи/чтения с разделяемой памяти.*/
    if((sem_id = semget(key, 2, O_RDWR)) == -1)
	{
		if((sem_id = semget(key, 2, 0660 | IPC_CREAT)) == -1) /* If server started first */
		{
			perror("semget");
			exit(EXIT_FAILURE);
		}
		/* Установить в семафорах №0 и №1 (Контроллеры ресурса) значение "1" */
		arg.val = 1;
		semctl(sem_id, 0, SETVAL, arg);
		semctl(sem_id, 1, SETVAL, arg);
	}

	/* Плучим дескриптор буфера разделяемой памяти */
	if((shm_id = shmget(key, MAX_BUF_SIZE, O_RDWR)) == -1)
	{
		if((shm_id = shmget(key, MAX_BUF_SIZE, IPC_CREAT | 0660)) == -1) /* If server started first */
		{
			perror("msgget");
			exit(EXIT_FAILURE);
		}
	}
	
	/* Получим доступ к разделяемой памяти */
	if (*(int*)(shm = (char*)shmat(shm_id, NULL, 0)) == -1)
	{
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	log_info("Waiting for a server...");
	/* Ждем записи разделяемой памяти сервером */
	if((semop(sem_id, &client_wait_for_server, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}
	log_info("OK");
	
	/* Ждем освобождения разделяемой памяти сервером, записывающим в память*/
	if((semop(sem_id, &serv_sem_lock, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}

	strncpy(str_for_receive, shm, MAX_BUF_SIZE);
	printf("This is string from server: %s\n", str_for_receive);

	/* Разблокируем разделяемую память */
	if((semop(sem_id, &serv_sem_unlock, 1)) == -1)
	{
		fprintf(stderr, "Unlock failed\n");
		exit(EXIT_FAILURE);
	}
	

	/* Заблокируем разделяемую память для записи сообщения серверу */
	if((semop(sem_id, &serv_sem_lock, 1)) == -1)
	{
		fprintf(stderr, "Lock failed\n");
		exit(EXIT_FAILURE);
	}

	strncpy(shm, str_for_send, MAX_BUF_SIZE);

	/* Разблокируем разделяемую память */
	if((semop(sem_id, &serv_sem_unlock, 1)) == -1)
	{
		fprintf(stderr, "Unlock failed\n");
		exit(EXIT_FAILURE);
	}

	/* Разблокируем разделяемую память семафором 2 для чтения сервером */
	if((semop(sem_id, &client_sem_unlock, 1)) == -1)
	{
		fprintf(stderr, "Unlock failed\n");
		exit(EXIT_FAILURE);
	}

	log_info("End of client program");
	exit(EXIT_SUCCESS);

#endif // SHARED_MEMORY_SYSTEM_V

#ifdef SHARED_MEMORY_POSIX
	
	mode_t mode = 0666;
	char *addr;

	/* Получим доступ к 2 семафорам - для синхронизации записи/чтения с разделяемой памяти. */
	if((sid1 = sem_open(sem_name1, 0)) == SEM_FAILED)
	{
		if((sid1 = sem_open(sem_name1, O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED) /* If server started first */
		{
			perror("sem_open1");
			exit(EXIT_FAILURE);
		}
	}
	if((sid2 = sem_open(sem_name2, 0)) == SEM_FAILED)
	{
		if((sid2 = sem_open(sem_name2, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED) /* If server started first */
		{
			perror("sem_open2");
			exit(EXIT_FAILURE);
		}
	}
	/* Создадим 3 семафор - для контроля последовательности запуска сервера и клиента */
	if((sid3 = sem_open(sem_name3, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
	{
		if((sid3 = sem_open(sem_name3, 0)) == SEM_FAILED) /* If server started first */
		{
			perror("sem_open2");
			exit(EXIT_FAILURE);
		}
	}

	if((shm_id = shm_open(shm_name, O_RDWR, 0)) == -1)
	{
		if((shm_id = shm_open(shm_name, O_CREAT | O_RDWR | O_EXCL, mode)) == -1) /* If server started first */
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
	if (ftruncate(shm_id, MAX_BUF_SIZE) == -1)
	{
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	if((addr = mmap(NULL, MAX_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED)
	{
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	log_info("Waiting for a server...");
	/* Ждем записи разделяемой памяти сервером */
	if(sem_wait(sid3) == -1)
	{
		perror("sem_wait2");
		exit(EXIT_FAILURE);
	}
	log_info("OK");

	/* Заблокируем разделяемую память для чтения сообщения от сервера */
	if(sem_wait(sid1) == -1)
	{
		perror("sem_wait1");
		exit(EXIT_FAILURE);
	}

	strncpy(str_for_receive, addr, MAX_BUF_SIZE);
	printf("This is string from server: %s\n", str_for_receive);

	/* Разблокируем разделяемую память */
	if(sem_post(sid1) == -1)
	{
		perror("sem_post1");
		exit(EXIT_FAILURE);
	}

	/* Заблокируем разделяемую память для чтения сообщения от сервера */
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

	/* Разблокируем разделяемую память семафором 2 для чтения сервером */
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

	if(close(shm_id) == -1)
	{
		perror("close shared mem fd");
		exit(EXIT_FAILURE);
	}

	log_info("End of client program");
	exit(EXIT_SUCCESS);
#endif // MSG_QUEUE_POSIX
}