#define _GNU_SOURCE
#ifndef __CURSED_MENU_H__
#define __CURSED_MENU_H__

#include "cursed_window.h"

#define KEY_ESC 27

void tui_make_menu (struct _cursed_window *win, char **promtList, int namesCounter);

void make_menu(struct _cursed_window  *window, char **promtList);

#endif // __CURSED_MENU_H__
