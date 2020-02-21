#define _GNU_SOURCE
#ifndef __CURSED_WINDOW_H__
#define __CURSED_WINDOW_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <panel.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
#include <fcntl.h>
#include <menu.h>

#define NUM_PROMTS  11
#define PROMT_LEN   128
#define NUM_WINDOWS 2

#define BACKGROUND_WINDOW    0
#define FOREGROUND_WINDOW    1

/* Структура для создания окон программы. Один объект - одно окно */
struct _cursed_window
{
    WINDOW *decoration;
    WINDOW *overlay;
    PANEL *panel;
};

/* Обработчик сигнала SIGWINCH */
void sig_winch(int signo);

/* Старт и первичная инициализация библиотеки ncurses */
void ncurses_start();

/* Инициализация цветовой палитры */
void init_colors();

/* Set window label */
void tui_win_label(WINDOW *win, char *string, int attrib);

/* Создание окна заднего фона программы - контейнера рабочих окон */
struct _cursed_window *tui_new_window(int sy, int sx, char *label);

/* Создание рабочего подокна программы */
struct _cursed_window *tui_new_subwindow(WINDOW *win, int sy, int sx, int h, int w);

void tui_progress_bar(WINDOW *win, double progress);

/* Создание всех окон программы */
void create_main_windows(struct _cursed_window **windows);

struct _cursed_window* create_help_window(struct _cursed_window *foregroundWindow, struct _cursed_window *helpWindow);

/* Read file and print content on main window */
struct _cursed_window* read_file(struct _cursed_window *foregroundWindow, int fd);

/* Refresh main window after creating service window */
void refresh_main_windows(struct _cursed_window **windows, int fd);

/* Служебное окно для отображения информации */
struct _cursed_window* create_service_window(struct _cursed_window *foregroundWindow, struct _cursed_window *serviceWindow, int h, int w, char *name);

/* Ставим панель на вершину стека панелей и обновляем */
void show_cursed_panel(struct _cursed_window *foregroundWindow);

/* Удаление сервисного окна */
void tui_del_service_window(struct _cursed_window *foregroundWindow);

/* Удаление всех окон программы и окончание работы с библиотекой ncurses*/
void tui_del_windows(struct _cursed_window **windows);

#endif // __struct _cursed_window_H__