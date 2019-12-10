#define _GNU_SOURCE
#ifndef __CURSED_MENU_H__
#define __CURSED_MENU_H__

#include <menu.h>
#include "cursed_window.h"


/* Keys, that don't works with codes in curses.h */
#define KEY_TAB     9
#define KEY_EENTER  10


char pathLeft[PATH_MAX];
char pathRight[PATH_MAX];
char resolved_path[PATH_MAX];

void free_items(ITEM **myItems, int namesCounter);

/* Получить имя каталога для первоначального запуска программы */
char* get_first_run_path();

/* Определить абсолютный путь выбранной папки */
char* resolve_path(char *sourcePath, const char *curItemDir);

/* Get all records within directory */
direntry** open_directory(direntry **nameList, char *path, int *namesCounter);

MENU *tui_make_menu (cursed_window *win,  ITEM **myItems, direntry **nameList, int namesCounter, char *path);

MENU* make_menu(cursed_window **windows, int winNum, MENU *myMenu, direntry **namelist, ITEM **myItems, char *path, int *namesCounter);

void keypad_on(cursed_window **windows);

void free_panel_menu(MENU *myMenu, direntry **namelist, ITEM **myItems, int *namesCounter);

#endif // __CURSED_MENU_H__
