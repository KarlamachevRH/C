#include "cursed_window.h"

/* Обработчик сигнала SIGWINCH */
void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
    resizeterm(size.ws_row, size.ws_col);
}

/* Старт и первичная инициализация библиотеки ncurses */
void ncurses_start()
{
    initscr();
    noecho(); /* Выключаем отображение вводимых символов (нужно для getch()) */
    cbreak(); 
    signal(SIGWINCH, sig_winch);
    curs_set(FALSE); /* Make the cursor invisible */
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
    use_default_colors();                       //Фон stscr будет "прозрачным"

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
    int i = 0, len = 0, j = 0, k = 0, height, width;
    char label[80] = {0};
    getmaxyx(win, height, width);
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
    mvwprintw(win, 1, ( width - strlen(string)) / 2, "%s", label);
    wattroff(win, attrib);
}

/* Создание окна заднего фона программы - контейнера рабочих окон */
cursed_window *tui_new_window(int sy, int sx, char *label)
{
    int h, w;
    getmaxyx(stdscr, h, w); 

    cursed_window *new = malloc (sizeof *new); 

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

/* Создание рабочего подокна программы для отображения списка файлов в каталоге */
cursed_window *tui_new_subwindow(WINDOW *win, int sy, int sx, int h, int w)
{
    cursed_window *new = malloc (sizeof(cursed_window));

    //Создаем окно для рамки относительно родительского окна    
    new->decoration = derwin(win, h, w, sy, sx);
    wbkgd(new->decoration, COLOR_PAIR(1));

    //Рисуем рамку
    box(new->decoration, 0, 0);
    int x, y;    
    getmaxyx(new->decoration, y, x);
    new->overlay = derwin(new->decoration, y-2, x-2, 1, 1);//рабочее дочернее окно
    wbkgd(new->overlay, COLOR_PAIR(1));
    new->panel = new_panel(new->decoration);

    //Даем команду обновить экран
    update_panels();
    doupdate();
    return new;
}

cursed_window *tui_new_service_window(WINDOW *win, int sy, int sx, int h, int w, char *label)
{
    cursed_window *new = malloc (sizeof(cursed_window));

    //Создаем окно для рамки относительно родительского окна    
    new->decoration = derwin(win, h, w, sy, sx);   

    //Рисуем рамку
    box(new->decoration, 0, 0);
    int x, y;    
    getmaxyx(new->decoration, y, x);
    new->overlay = derwin(new->decoration, y-2, x-2, 1, 1);//рабочее дочернее окно
    
    new->panel = new_panel(new->decoration);
    tui_win_label(new->decoration, label, 0);
    update_panels();
    doupdate(); 
    return new;
}

/* Создание всех окон программы */
void create_windows(cursed_window **windows)
{      
    int subWndHeight, subWndWidth;    

    //Создадим окна программы
    windows[BACKGROUND_PANEL] = tui_new_window(0, 0, "-=File Commander. F10 - exit, Tab - change panel=-");

    getmaxyx(windows[0]->overlay, subWndHeight, subWndWidth);

    windows[LEFT_PANEL] = tui_new_subwindow(windows[BACKGROUND_PANEL]->overlay, 1, 1, subWndHeight - 2, subWndWidth/2 - 1);
    windows[RIGHT_PANEL] = tui_new_subwindow(windows[BACKGROUND_PANEL]->overlay, 1, subWndWidth/2, subWndHeight - 2, subWndWidth/2 - 1); 
}

cursed_window* create_service_window(cursed_window **windows, cursed_window *win)
{    
    int height, width;    
    getmaxyx(windows[BACKGROUND_PANEL]->overlay, height, width);
    win = tui_new_service_window(windows[BACKGROUND_PANEL]->overlay, height/3, width/4, 10, 60, "Copying files. Progress:");
    return win;  
}

void tui_progress_bar(WINDOW *win, double progress)
{
    int  height, width;
    getmaxyx(win, height, width);
    wattron(win, COLOR_PAIR(8));
    for (int  i = 0; i < width * progress; i++)
    {
        mvwaddch(win, (height / 2), i, ' ');
    }
    wattroff(win, COLOR_PAIR(8));
    wattron(win, COLOR_PAIR(7));
    for (int  i = width * progress; i < width; i++)
    {
        mvwaddch(win, (height / 2), i, ' ');
    }
    wattroff(win, COLOR_PAIR(7));
    wattron(win, A_STANDOUT);
    mvwprintw(win, (height / 2), (width / 2) - 2, "%.0lf%%", progress*100);
    wattroff(win, A_STANDOUT);
}

/* Ставим панель на вершину стека панелей и обновляем */
void update_cursed_panel(cursed_window **windows, int panelNum)
{
    PANEL *TOP;    
    TOP = windows[panelNum]->panel; 
    top_panel(TOP);
    touchwin(panel_window(TOP));
    update_panels();
    doupdate();
}

void update_service_panel(cursed_window *window)
{
    PANEL *TOP;    
    TOP = window->panel; 
    top_panel(TOP);
    touchwin(panel_window(TOP));
    update_panels();
    doupdate();
}

/* Удаление всех окон программы и окончание работы с библиотекой ncurses*/
void tui_del_windows(cursed_window **windows)
{
    for (int i = NUM_WINDOWS-1; i >= 0; i--)
    {
        del_panel(windows[i]->panel);
	    delwin(windows[i]->overlay);
        delwin(windows[i]->decoration);
    }
    endwin(); // End programm    
}

void tui_del_service_window(cursed_window *window)
{
    del_panel(window->panel);
    delwin(window->overlay);
    delwin(window->decoration);
}