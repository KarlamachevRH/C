#ifndef __FORK_PROCCESS_H__
#define __FORK_PROCCESS_H__
#define _GNU_SOURCE

#include <wait.h>
#include "cursed_menu.h"

int fork_programm(const char *namelist);

#endif // __FORK_PROCCESS_H__