#include "headers.h"


#ifdef SHARED_MEMORY_SYSTEM_V

void sem_init()
{
	/* Блокировка буфера сервером */
	serv_sem_lock.sem_num = 0;
	serv_sem_lock.sem_op = -1;
	serv_sem_lock.sem_flg = 0;

	/* Освобождение буфера сервером */
	serv_sem_unlock.sem_num = 0;
	serv_sem_unlock.sem_op = 1;
	serv_sem_unlock.sem_flg = 0;

	client_wait_for_server.sem_num = 1;
	client_wait_for_server.sem_op = 0;
	client_wait_for_server.sem_flg = 0;	

	/* Разблокировка буфера клиентом после записи сообщения в буфер */
	client_sem_unlock.sem_num = 1;
	client_sem_unlock.sem_op = 1;
	client_sem_unlock.sem_flg = 0;

	/* Блокировка буфера сервером после чтения сообщения от клиента */
	client_sem_lock.sem_num = 1;
	client_sem_lock.sem_op = -1;
	client_sem_lock.sem_flg = 0;
}

#endif // SHARED_MEMORY_SYSTEM_V

#ifdef SHARED_MEMORY_POSIX
/* No data */
#endif // SHARED_MEMORY_POSIX