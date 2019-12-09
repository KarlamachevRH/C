#define _GNU_SOURCE
#ifndef __CURSED_WINDOW_H__
#define __CURSED_WINDOW_H__

#include <panel.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "mem_managment.h"

#define NUM_WINDOWS         3
#define BACKGROUND_PANEL    0
#define LEFT_PANEL          1
#define RIGHT_PANEL         2

/* Структура для создания окон программы. Один объект - одно окно */
struct _cursed_window
{    
    WINDOW *decoration;  
    WINDOW *overlay;    
    PANEL *panel;
};
/* Для упрощения определения создадим новое имя для структуры */
typedef struct _cursed_window cursed_window;

/* Обработчик сигнала SIGWINCH */
void sig_winch(int signo);

/* Старт и первичная инициализация библиотеки ncurses */
void ncurses_start();

/* Инициализация цветовой палитры */
void init_colors();

/* Set window label */
void tui_win_label(WINDOW *win, char *string, int attrib);

/* Создание окна заднего фона программы - контейнера рабочих окон */
cursed_window *tui_new_window(int sy, int sx, char *label);

/* Создание рабочего подокна программы для отображения списка файлов в каталоге */
cursed_window *tui_new_subwindow(WINDOW *win, int sy, int sx, int h, int w);

void tui_progress_bar(WINDOW *win, double progress);

/* Создание всех окон программы */
void create_windows(cursed_window **windows);

/* Служебное окно для отображения информации */
cursed_window* create_service_window(cursed_window **windows, cursed_window *win);

/* Ставим панель на вершину стека панелей и обновляем */
void update_cursed_panel(cursed_window **windows, int panelNum);

void update_service_panel(cursed_window *window);

/* Удаление сервисного окна */
void tui_del_service_window(cursed_window *window);

/* Удаление всех окон программы и окончание работы с библиотекой ncurses*/
void tui_del_windows(cursed_window **windows);

#endif // __CURSED_WINDOW_H__