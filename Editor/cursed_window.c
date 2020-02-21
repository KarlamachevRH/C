#include "cursed_window.h"

/* Обработчик сигнала SIGWINCH */
void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *)&size);
    resizeterm(size.ws_row, size.ws_col);
}

/* Старт и первичная инициализация библиотеки ncurses */
void ncurses_start()
{
    initscr();
    noecho(); /* Выключаем отображение вводимых символов (нужно для getch()) */
    cbreak(); 
    signal(SIGWINCH, sig_winch);
    //curs_set(FALSE); /* Make the cursor invisible */
    nonl ();
    init_colors();
}

/* Инициализация цветовой палитры */
void init_colors()
{
    if (has_colors() == FALSE) /* If colors not supported */ 
    {
        endwin();
        puts("\nYour terminal does not support color");
        return;
    }
    start_color();                              //Активируем поддержку цвета
    use_default_colors();                       //Фон stdscr будет прозрачным

    init_pair( 1, COLOR_WHITE,   COLOR_BLUE);
    init_pair( 2, COLOR_GREEN,    COLOR_BLACK);
    init_pair( 3, COLOR_MAGENTA,  COLOR_BLUE);
    init_pair( 4, COLOR_BLACK,     COLOR_YELLOW);
    init_pair( 5, COLOR_GREEN,   COLOR_WHITE);
    init_pair( 6, COLOR_BLUE,   COLOR_WHITE);
    init_pair( 7, COLOR_MAGENTA,   COLOR_WHITE);
    init_pair( 8, COLOR_WHITE, COLOR_MAGENTA);
    init_pair( 9, COLOR_CYAN,    COLOR_WHITE);
    init_pair(10, COLOR_YELLOW,  COLOR_BLACK);
}

/* Set window label */
void tui_win_label(WINDOW *win, char *string, int attrib)
{
    if ( !string[0] )
        return;
    if ( attrib == 0 )
        attrib = A_NORMAL;
    //Draw borders
    int i = 0, len = 0, j = 0, k = 0, width;
    char label[80] = {0};
    width = getmaxx(win);
    mvwhline(win, 2, 1, ACS_HLINE, width - 2);
    char clearw[128] = {' '};
    clearw[width - 1] = 0;
    mvwprintw(win, 1, 1, clearw);

    len = strlen(string);
    if ( len > width )
        i = len - width;
    else
        i = 0;
    for ( j = i; j < len; j++ )
    {
        label[k] = string[j];
        k++;
    }
    label[k + 1] = '\0';
    if ( len > width )
        label[0] = '~';
    wattron(win, attrib);
    mvwprintw(win, 1, (width - strlen(string))/2, "%s", label);
    wattroff(win, attrib);
}

/* Создание окна заднего фона программы - контейнера рабочего окна */
struct _cursed_window *tui_new_window(int sy, int sx, char *label)
{
    int h, w;
    getmaxyx(stdscr, h, w); 

    struct _cursed_window *new = malloc (sizeof *new); 

    // Создаем окно для рамки
    new->decoration = newwin(h, w, sy, sx);
    wbkgd(new->decoration, COLOR_PAIR(1));
    //Рисуем рамку
    box(new->decoration, 0, 0);
    int x, y;
    getmaxyx(new->decoration, y, x);
    new->overlay = derwin(new->decoration, y-2, x-2, 2, 1); //рабочее дочернее окно 
    wbkgd(new->overlay, COLOR_PAIR(1));
    new->panel = new_panel(new->decoration);    
    tui_win_label(new->decoration, label, 0);
    //Даем команду обновить экран
    update_panels();
    doupdate();
    return new;
}

/* Создание рабочего подокна программы для отображения содержимого файла */
struct _cursed_window *tui_new_subwindow(WINDOW *win, int sy, int sx, int h, int w)
{
    struct _cursed_window *new = malloc (sizeof(struct _cursed_window));

    //Создаем окно для рамки относительно родительского окна    
    new->decoration = derwin(win, h, w, sy, sx);
    wbkgd(new->decoration, COLOR_PAIR(1));

    int x, y;
    getmaxyx(new->decoration, y, x);
    new->overlay = derwin(new->decoration, y-2, x-2, 0, 0);//рабочее дочернее окно
    wbkgd(new->overlay, COLOR_PAIR(1));
    new->panel = new_panel(new->decoration);

    //Даем команду обновить экран
    update_panels();
    doupdate();
    return new;
}

struct _cursed_window *tui_new_service_window(WINDOW *win, int sy, int sx, int h, int w, char *label)
{
    struct _cursed_window *new = malloc(sizeof(struct _cursed_window));

    //Создаем окно для рамки относительно родительского окна
    new->decoration = derwin(win, h, w, sy, sx);
    wbkgd(new->decoration, COLOR_PAIR(5));
    //Рисуем рамку
    box(new->decoration, 0, 0);
    int x, y;
    getmaxyx(new->decoration, y, x);
    new->overlay = derwin(new->decoration, y-2, x-2, 1, 1); //рабочее дочернее окно
    wbkgd(new->overlay, COLOR_PAIR(5));
    new->panel = new_panel(new->decoration);
    tui_win_label(new->decoration, label, 0);
    return new;
}

struct _cursed_window* create_service_window(struct _cursed_window *window, struct _cursed_window *serviceWindow, int h, int w, char *name)
{    
    int height, width;
    getmaxyx(window->overlay, height, width);
    serviceWindow = tui_new_service_window(window->overlay, height/5, width/3.5, h, w, name);
    return serviceWindow;
}

/* Создание всех окон программы */
void create_main_windows(struct _cursed_window **windows)
{
    int subWndHeight, subWndWidth;
    windows[BACKGROUND_WINDOW] = tui_new_window(0, 0, "-=Editor. F2 - Help=-");
    getmaxyx(windows[BACKGROUND_WINDOW]->overlay, subWndHeight, subWndWidth);
    windows[FOREGROUND_WINDOW] = tui_new_subwindow(windows[BACKGROUND_WINDOW]->overlay, 1, 1, subWndHeight - 2, subWndWidth - 1);
}

struct _cursed_window* read_file(struct _cursed_window *window, int fd)
{
    int line = 0;
    char c;
    int size = -1;
    while (size != 0 && c != EOF && line < LINES - 2)
    {
        if((size = read(fd, &c, sizeof(char))) < 0)
        {
            perror("Can't find text in file!");
            break;
        }
        if (c == '\n')
            line++;
        waddch(window->overlay, c);
    }
    lseek(fd, SEEK_SET, 0); //вернемся в начало файла (для новой загрузки данных)
    idlok(window->overlay, TRUE);
    scrollok(window->overlay, TRUE);
    wmove(window->overlay, 0, 0);
    wrefresh(window->overlay);
    return window;
}

struct _cursed_window* create_help_window(struct _cursed_window *foregroundWindow, struct _cursed_window *helpWindow)
{
    int mainWindowHight = 0;
    int mainWindowWidth = 0;
    getmaxyx(foregroundWindow->overlay, mainWindowHight, mainWindowWidth);
    helpWindow = create_service_window(foregroundWindow, helpWindow, mainWindowHight/3, mainWindowWidth/1.8, "Help");
    return helpWindow;
}

void refresh_main_windows(struct _cursed_window **windows, int fd)
{
    tui_del_windows(windows);
    create_main_windows(windows);
    windows[FOREGROUND_WINDOW] = read_file(windows[FOREGROUND_WINDOW], fd);
    show_cursed_panel(windows[FOREGROUND_WINDOW]);
}

/* Ставим панель на вершину стека панелей и обновляем */
void show_cursed_panel(struct _cursed_window *window)
{
    PANEL *TOP;
    TOP = window->panel; 
    top_panel(TOP);
    touchwin(panel_window(TOP));
    update_panels();
    doupdate();
}

/* Удаление всех окон программы и окончание работы с библиотекой ncurses*/
void tui_del_windows(struct _cursed_window **windows)
{
    for (int i = NUM_WINDOWS-1; i >= 0; i--)
    {
        del_panel(windows[i]->panel);
	    delwin(windows[i]->overlay);
        delwin(windows[i]->decoration);
    }
    endwin(); // End programm
}

void tui_del_service_window(struct _cursed_window *window)
{
    del_panel(window->panel);
    delwin(window->overlay);
    delwin(window->decoration);
}