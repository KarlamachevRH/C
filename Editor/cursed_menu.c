#include "cursed_menu.h"

void tui_make_menu (struct _cursed_window *win, char **promtList, int promtNum)
{
    int h = getmaxy(win->overlay);
    if(h < 0)
    {
        perror("tui_make_menu: getmaxy() from main window");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < promtNum; i++)
        mvwprintw(win->overlay, i+3, 1, "%s\n", promtList[i]);
}

void make_menu(struct _cursed_window  *helpWindow, char **promtList)
{
    tui_make_menu(helpWindow, promtList, NUM_PROMTS);
    show_cursed_panel(helpWindow);
}