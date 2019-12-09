#include "cursed_menu.h"

void free_items(ITEM **myItems, int namesCounter)
{    
    if(myItems != NULL)
    {
        for(int i = 0; i < namesCounter; i++)
        {
            if(myItems[i] != NULL)
                free_item(myItems[i]);
        }       
    }    
}

/* Получить имя каталога для первоначального запуска программы */
char* get_first_run_path()
{
    char *path = getenv("HOME"); // получить значение переменной окружения HOME  
    if (path == NULL)
        path = "/";
    return path;    
}

/* Определить абсолютный путь выбранной папки */
char* resolve_path(char *sourcePath, const char *curItemDir)
{    
    strncpy(sourcePath, "./", PATH_MAX);
    strncat(sourcePath, curItemDir, PATH_MAX);                                
    if((realpath(sourcePath, resolved_path)) == NULL)
    {
        perror("Can't resolve path");
        exit(errno);
    }
    strncpy(sourcePath, resolved_path, PATH_MAX);
    return sourcePath;
}

/* Get all records within directory */
direntry** open_directory(direntry **nameList, char *path, int *namesCounter)
{
    *namesCounter = scandir(path, &nameList, 0, alphasort);
    if (*namesCounter < 0)
        perror("scandir");
    return nameList;
}

MENU *tui_make_menu (cursed_window *win,  ITEM **myItems, direntry **nameList, int namesCounter, char *path)
{
    MENU *menu;
    int h, w;
    getmaxyx(win->overlay, h, w);
    int i, j = 1;
    
    if(strcmp(resolved_path, "/") == 0) // path is root directory
        j = 2;
    
    for(i = 0; j < namesCounter; i++, j++)
        myItems[i] = new_item(nameList[j]->d_name, "");
    myItems[i] = NULL; // we need NULL terminated items list, else we get error on next step
    
    if(!(menu = new_menu((ITEM **)myItems)))
    {
        perror("New directory list creating");
        exit(errno);
    }
    set_menu_win(menu, win->decoration);
    set_menu_sub(menu, win->overlay);
    set_menu_format(menu, h, 1);    
    set_menu_mark(menu, " ");
    set_menu_fore(menu, COLOR_PAIR(1) | A_REVERSE);
    set_menu_back(menu, COLOR_PAIR(1));
    post_menu(menu);
    return menu;
}
