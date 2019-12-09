#include "fork_proccess.h"
#include "pthread_create.h"


int main(int argc, char ** argv)
{   
    ncurses_start();
    
    /* create windows for output dir list of content */
    cursed_window *windows[NUM_WINDOWS];    
    create_windows(windows);

    direntry **namelistLeft = NULL;      /* Массив для имен файлов и каталогов в текущем каталоге */
    direntry **namelistRight = NULL;
    int namesCounterLeft;
    int namesCounterRight;
    struct stat filestat;
    MENU *myMenuLeft = NULL;
    MENU *myMenuRight = NULL;
    char pathFrom[PATH_MAX] = {0};
    char pathTo[PATH_MAX] = {0};

    strncpy(pathLeft, get_first_run_path(), PATH_MAX);    
    strncpy(pathRight, get_first_run_path(), PATH_MAX);
    namelistRight = open_directory(namelistRight, pathRight, &namesCounterRight);

    ITEM **myItemsLeft = NULL;
    ITEM **myItemsRight = (ITEM **)calloc(namesCounterRight, sizeof(ITEM *));

    myMenuRight = tui_make_menu(windows[RIGHT_PANEL], myItemsRight, namelistRight, namesCounterRight, pathRight);
    update_cursed_panel(windows, BACKGROUND_PANEL);
    
    int key = 0;
    int result;
    int changePanel = 0;

    keypad(windows[LEFT_PANEL]->overlay, TRUE);
    keypad(windows[RIGHT_PANEL]->overlay, TRUE);
        
    while(key != KEY_F(10))
    {
        if(!changePanel && key != KEY_F(10)) // left panel
        {
            key = 0;
            namesCounterLeft = 0;
            namelistLeft = open_directory(namelistLeft, pathLeft, &namesCounterLeft);
            if((myItemsLeft = (ITEM **)calloc(namesCounterLeft, sizeof(ITEM *))) == NULL)
                exit(EXIT_FAILURE);
                    
            myMenuLeft = tui_make_menu (windows[LEFT_PANEL], myItemsLeft, namelistLeft, namesCounterLeft, pathLeft);
            update_cursed_panel(windows, BACKGROUND_PANEL);

            while(key != KEY_F(10) && key != KEY_TAB && key != KEY_EENTER)
            {
                key = wgetch(windows[LEFT_PANEL]->overlay);
                switch(key)
                {
                    case KEY_UP:
                        menu_driver(myMenuLeft, REQ_UP_ITEM);
                        break;
                    case KEY_DOWN: 
                        menu_driver(myMenuLeft, REQ_DOWN_ITEM);
                        break;
                    case KEY_NPAGE: 
                        menu_driver(myMenuLeft, REQ_SCR_DPAGE);
                        break;
                       case KEY_PPAGE:
                        menu_driver(myMenuLeft, REQ_SCR_UPAGE);
                        break;
                    case KEY_TAB:
                        changePanel = 1;
                        strncpy(resolved_path, pathRight, PATH_MAX); 
                        break;
                    case KEY_EENTER:
                        chdir(pathLeft);  // перемещаемся в указанную директорию
                        result = lstat(myMenuLeft->curitem->name.str, &filestat);
                        if(result == 0)
                        {
                            if(S_ISDIR(filestat.st_mode)) // If choice is directory - copy name to path buf
                                resolve_path(pathLeft, myMenuLeft->curitem->name.str);
                            else 
                                if(S_IXUSR & filestat.st_mode)
                                    fork_programm(myMenuLeft->curitem->name.str);
                        }
                        break;
                    case KEY_F(4):
                        /* Пути для копирования файлов */
                        pathFrom[0] = 0;
                        pathTo[0] = 0;
                        strncpy(pathFrom, pathLeft, PATH_MAX);
                        strncat(pathFrom, "/", PATH_MAX);
                        strncat(pathFrom, myMenuLeft->curitem->name.str, PATH_MAX);
                        strncat(pathTo, pathRight, PATH_MAX);
                        strncat(pathTo, "/", PATH_MAX);
                        strncat(pathTo, myMenuLeft->curitem->name.str, PATH_MAX);

                        /* Создадим потоки копирования и отображения прогресса копирования */
                        create_threads(pathFrom, pathTo, windows);
                        //update_cursed_panel(windows, BACKGROUND_PANEL);                        
                        break;
                    default:
                        break;
                }
            }
        }
        
        if(changePanel && key != KEY_F(10)) // right panel
        {
            key = 0;
            unpost_menu(myMenuRight);
            free_menu(myMenuRight);
            free_items(myItemsRight, namesCounterRight);
            free_mem(namelistRight, namesCounterRight);
            namesCounterRight = 0;

            namelistRight = open_directory(namelistRight, pathRight, &namesCounterRight);
            if(!(myItemsRight = (ITEM **)calloc(namesCounterRight, sizeof(ITEM *))))
                exit(EXIT_FAILURE);

            myMenuRight = tui_make_menu (windows[RIGHT_PANEL], myItemsRight, namelistRight, namesCounterRight, pathRight);
            update_cursed_panel(windows, BACKGROUND_PANEL);

            while(key != KEY_F(10) && key != KEY_TAB && key != KEY_EENTER)
            {            
                key = wgetch(windows[RIGHT_PANEL]->overlay);
                switch(key)
                {
                    case KEY_UP:
                        menu_driver(myMenuRight, REQ_UP_ITEM);
                        break;
                    case KEY_DOWN:
                        menu_driver(myMenuRight, REQ_DOWN_ITEM);
                        break;
                    case KEY_NPAGE:
                        menu_driver(myMenuRight, REQ_SCR_DPAGE);
                        break;
                    case KEY_PPAGE:
                        menu_driver(myMenuRight, REQ_SCR_UPAGE);
                        break;
                    case KEY_TAB:
                        changePanel = 0;
                        strncpy(resolved_path, pathLeft, PATH_MAX);
                        break;
                    case KEY_EENTER:
                        chdir(pathRight);  // перемещаемся в указанную директорию
                        result = lstat(myMenuRight->curitem->name.str, &filestat);
                        if(result == 0)
                        {
                            if(S_ISDIR(filestat.st_mode)) // If choice is directory - copy name to path buf
                                resolve_path(pathRight, myMenuRight->curitem->name.str);
                            else 
                                if(S_IXUSR & filestat.st_mode)
                                    fork_programm(myMenuRight->curitem->name.str);
                        }
                        break;
                    case KEY_F(4):
                        /* Пути для копирования файлов */
                        pathFrom[0] = 0;
                        pathTo[0] = 0;
                        strncpy(pathFrom, pathRight, PATH_MAX);
                        strncat(pathFrom, myMenuRight->curitem->name.str, PATH_MAX);
                        strncat(pathTo, pathLeft, PATH_MAX);
                        strncat(pathTo, myMenuRight->curitem->name.str, PATH_MAX);

                        /* Создадим потоки копирования и отображения прогресса копирования */
                        create_threads(pathFrom, pathTo, windows);
                        //update_cursed_panel(windows, BACKGROUND_PANEL);
                        break;
                    default:
                        break;
                }            
            }
        }
        if(!changePanel) // unpost left panel menu here because we need it before update left panel
        {   
            unpost_menu(myMenuLeft);
            free_menu(myMenuLeft);
            free_items(myItemsLeft, namesCounterLeft);
            free_mem(namelistLeft, namesCounterLeft);
            namesCounterLeft = 0;
        }
    }
    tui_del_windows(windows);
    return EXIT_SUCCESS;
}