#include "cursed_menu.h"

/* Глобальное значение текущего положения курсора */
int row, col;

int open_file(const char *path, int oflag, int fd)
{
    if((fd = open(path, oflag, 0666)) < 0)
    {
        perror("Can't open file");
        close(fd);
        return -1;
    }
    return fd;
}

char* show_file(char *path, struct _cursed_window **mainWindows, int fd)
{
    struct _cursed_window *pathWindow = NULL;
    close(fd); //предыдущий файл закроем
    pathWindow = create_service_window(mainWindows[FOREGROUND_WINDOW], pathWindow, 40, 70, "Enter file name");
    keypad(pathWindow->overlay, TRUE);
    show_cursed_panel(pathWindow);
    wgetstr(pathWindow->overlay, path);
    tui_del_service_window(pathWindow);
    fd = open_file(path, O_RDONLY | O_CREAT, fd);
    refresh_main_windows(mainWindows, fd);
    return path;
}

int line_len(struct _cursed_window *mainWindow, int lineNum)
{
    int len = COLS - 1;

    while (len >= 0 && mvwinch(mainWindow->overlay, lineNum, len) == ' ')
        len--;
    return len + 1;
}

void save_file(struct _cursed_window *mainWindow, char *path)
{
    int i, n, l;
    FILE* fp = fopen(path, "w");
    for (l = 0; l < LINES - 1; l++)
    {
        n = line_len(mainWindow, l);
        for (i = 0; i < n; i++)
            putc(mvwinch(mainWindow->overlay, l, i) & A_CHARTEXT, fp);
        putc('\n', fp);
    }
    fclose(fp);
}

void input(WINDOW *w)
{
    int c;
    wstandout(w);
    mvwaddstr(w, LINES - 1, COLS - 20, "Режим ввода");
    wstandend(w);
    wmove(w, row, col);
    wrefresh(w);
    for (;;) 
    {
        c = wgetch(w);
        if (c == CTRL('D') || c == KEY_EIC) 
            break;
        winsch(w, c);
        wmove(w, row, ++col);
        wrefresh(w);
    }
    wmove (w, LINES - 1, COLS - 20);
    wclrtoeol(w);
    wmove (w, row, col);
    wrefresh(w);
}

void create_help_list(struct _cursed_window *helpWindow)
{
    char *promtList[NUM_PROMTS] =       /* Массив для списка подсказок */
    {
        "F3 - input name/path to open file",
        "F2 - show help",
        "ESC - close help or exit from input mode",
        "i - input mode",
        "hjkl and arrows - move the cursor in the specified direction",
        "x - delete symbol under the cursor",
        "o - input string and go to input mode",
        "d - delete current line",
        "CTRL+L - redraw the screen",
        "w - save file and exit",
        "q - exit without saving file",
    };
    make_menu(helpWindow, promtList);
}

/* */
void edit(struct _cursed_window **mainWindows, char *path, int fd)
{
    int c;
    int e = 0;

    struct _cursed_window *helpWindow = NULL;
    
    for (; e == 0; )
    {
        wmove(mainWindows[FOREGROUND_WINDOW]->overlay, row, col);
        wrefresh(mainWindows[FOREGROUND_WINDOW]->overlay);

        c = wgetch(mainWindows[FOREGROUND_WINDOW]->overlay);

        /* Команды редактора */
        switch(c)
        {
            case KEY_F(2):
                keypad(mainWindows[FOREGROUND_WINDOW]->overlay, FALSE);
                helpWindow = create_help_window(mainWindows[FOREGROUND_WINDOW], helpWindow);
                keypad(helpWindow->overlay, TRUE);
                create_help_list(helpWindow);
                while(c != KEY_ESC)
                    c = wgetch(helpWindow->overlay);
                keypad(helpWindow->overlay, FALSE);
                tui_del_service_window(helpWindow);
                refresh_main_windows(mainWindows, fd);
                keypad(mainWindows[FOREGROUND_WINDOW]->overlay, TRUE);
                break;

            case KEY_F(3):
                path = show_file(path, mainWindows, fd);
                break;

            /* hjkl и стрелки: перемещают курсор
            в указанном направлении */
            case 'h':
            case KEY_LEFT:
                if (col > 0) 
                    col--;
                else 
                    flash();
                break;

            case 'j':
            case KEY_DOWN:
                if (row < LINES - 1) 
                    row++;
                else 
                    flash();
                break;

            case 'k':
            case KEY_UP:
                if (row > 0) 
                    row--;
                else 
                    flash ();
                break;

            case 'l':
            case KEY_RIGHT:
                if (col < COLS - 1) 
                    col++;
                else 
                    flash();
                break;

            /* i: переход в режим ввода */
            case KEY_IC:
            case 'i':
                input(mainWindows[FOREGROUND_WINDOW]->overlay);
                break;
            
            case KEY_BACKSPACE:
                if (col > 0) 
                    col--;
                mvwdelch(mainWindows[FOREGROUND_WINDOW]->overlay, row, col);
                break;

            /* x: удалить текущий символ */
            case KEY_DC:
            case 'x':
                wdelch(mainWindows[FOREGROUND_WINDOW]->overlay);
                break;

            /* o: вставить строку и перейти в режим ввода */
            case KEY_IL:
            case 'o':
                wmove(mainWindows[FOREGROUND_WINDOW]->overlay, ++row, col = 0);
                winsdelln(mainWindows[FOREGROUND_WINDOW]->overlay, 1);
                input(mainWindows[FOREGROUND_WINDOW]->overlay);
                break;

            /* d: удалить текущую строку */
            case KEY_DL:
            case 'd':
                winsdelln(mainWindows[FOREGROUND_WINDOW]->overlay, -1);
                break;

            /* CTRL+L: перерисовать экран */
            case KEY_CLEAR:
            case CTRL('L'):
                wrefresh(mainWindows[FOREGROUND_WINDOW]->overlay);
                break;

            /* w: записать изменения в файл */
            case 'w':
                close(fd);
                save_file(mainWindows[FOREGROUND_WINDOW], path);
                break;

            /* q: закончить работу без записи файла */
            case 'q':
                free(path);
                close(fd);
                e = 1;
                break;

            default:
                flash();
                break;
        }
    }
}

/* Режим ввода: принимает и вставляет символы
    Выход: CTRL+D или EIC */
int main(int argc, char ** argv)
{
    int fd = -1;
    struct _cursed_window *mainWindows[NUM_WINDOWS];
    char *path = calloc(PATH_MAX, sizeof(char));

    //setlocale(LC_ALL, "");

    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s path/to/file\n", argv [0]);
        exit(EXIT_FAILURE);
    }

    fd = open_file(argv[1], O_RDONLY | O_CREAT, fd);
    if(fd < 0)
        exit(EXIT_FAILURE);
    strncpy(path, argv[1], PATH_MAX);

    ncurses_start();

    create_main_windows(mainWindows);

    keypad(mainWindows[FOREGROUND_WINDOW]->overlay, TRUE);

    /* Читаем файл */
    mainWindows[FOREGROUND_WINDOW] = read_file(mainWindows[FOREGROUND_WINDOW], fd);
    show_cursed_panel(mainWindows[FOREGROUND_WINDOW]);

    edit(mainWindows, path, fd);
    
    tui_del_windows(mainWindows);
    return EXIT_SUCCESS;
}