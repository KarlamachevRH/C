#include "pthread_create.h"

struct 
{
	pthread_mutex_t	mutex;
	unsigned long long bytes_done;	/* Size of copied data for threads synchronization */
} shared = 
{ 
	PTHREAD_MUTEX_INITIALIZER,
    BYTES_DONE
};

unsigned long long calculate_size(char *path)
{
    DIR *d = NULL;
    unsigned long long size = 0;
    struct dirent *fname;
    struct stat filestat;
    int result;
    char readedPath[PATH_MAX] = {0};
    strncat(path, "/", PATH_MAX);
    strncpy(readedPath, path, PATH_MAX);

    if ((d = opendir(path))) 
    {
        while((fname = readdir(d)))
        {
            if (!strcmp(fname->d_name, ".") || !strcmp(fname->d_name, ".."))
                continue;            
            else 
            {                
                strncat(readedPath, fname->d_name, PATH_MAX);
                result = lstat(readedPath, &filestat);
                if(result == 0)
                {
                    if(S_ISDIR(filestat.st_mode))
                        size += calculate_size(readedPath);
                    else
                        if(S_ISREG(filestat.st_mode))
                            size += filestat.st_size;
                }
                else
                {
                    perror("calculate_size: get file stat");
                    pthread_exit((void*)&result);
                }   
                strncpy(readedPath, path, PATH_MAX);                                                
            }            
        }        
        closedir(d);
    }
    else
    {
        perror("calculate_size: opendir");
        pthread_exit((void*)&errno);
    }
    return size;
}

void copy_file(char *file_in, char *file_out)
{
    char content[BUFF_SIZE];
    
    int fin;
    int fout;

    if( (fin = open(file_in, O_RDONLY)) < 0 )
    {
        perror("Open for read of file_in failed");
        pthread_exit((void*)&errno);
    }
    //printf("file_out: %s\n", file_out);
    if( (fout = open(file_out, O_WRONLY | O_CREAT, S_IWRITE | S_IREAD)) < 0 )
    {
        perror("Open for write of file_out failed");
        close(fin);
        pthread_exit((void*)&errno);
    }
    int readCount = 0;
    while( (readCount = read(fin, content, sizeof(content))) > 0 )
    {
        if( readCount != write(fout, content, readCount) )
        {
            perror( "Write of data to output file failed" );
            close(fin);
            close(fout);
            pthread_exit((void*)&errno);
        }
        /* If we writed the data block, increase bytes counter */
        pthread_mutex_lock(&shared.mutex);
        shared.bytes_done += readCount;
        pthread_mutex_unlock(&shared.mutex);
    }
    if(readCount < 0)
    {
        perror( "Read of file failed" );
        close(fin);
        close(fout);
        pthread_exit((void*)&errno);
    }
    close(fin);
    close(fout);
}

void copy(char *pathFrom, char *pathTo)
{
    int result;
    struct stat filestat;
    struct dirent *fname;
    char file_in[PATH_MAX] = {0}, file_out[PATH_MAX] = {0};
    char tempBufIn[PATH_MAX] = {0};
    char tempBufOut[PATH_MAX] = {0};
    DIR *d = NULL;

    result = lstat(pathFrom, &filestat);    
    if(S_ISDIR(filestat.st_mode))
    {        
        if((d = opendir(pathFrom)) == NULL)
        {
            perror("copy: opendir");
            pthread_exit((void*)&errno);
        }
        strncpy(file_in, pathFrom, PATH_MAX);
        strncpy(file_out, pathTo, PATH_MAX);
        strncat(file_in, "/", PATH_MAX);
        strncat(file_out, "/", PATH_MAX);
        /* Create directory in directory to copy */                        
        if(mkdir(file_out, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
        {
            perror("Creating directory");
            pthread_exit((void*)&errno);
        }
        strncpy(tempBufIn, file_in, PATH_MAX);
        strncpy(tempBufOut, file_out, PATH_MAX);
        while((fname = readdir(d)))
        {
            if (!strcmp(fname->d_name, ".") || !strcmp(fname->d_name, ".."))
                continue;
            else
            {                
                strncat(file_in, fname->d_name, PATH_MAX);
                result = lstat(file_in, &filestat);
                if(result == 0)
                {
                    if(S_ISDIR(filestat.st_mode))
                    {                                                
                        strncat(file_out, "/", PATH_MAX);
                        strncat(file_out, fname->d_name, PATH_MAX);

                        /* Create directory in directory to copy */                        
                        if(mkdir(file_out, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
                        {
                            perror("Creating directory");
                            pthread_exit((void*)&errno);
                        }
                        copy(file_in, file_out);
                    }
                    else 
                        if(S_ISREG(filestat.st_mode))
                        {
                            strncat(file_out, fname->d_name, PATH_MAX);
                            copy_file(file_in, file_out);
                        }
                }
                else
                {
                    perror("copy: get file stat");
                    pthread_exit((void*)&result);
                }
                strncpy(file_in, tempBufIn, PATH_MAX);
                strncpy(file_out, tempBufOut, PATH_MAX);
            }
        }
        closedir(d);
    }
    else    
        copy_file(pathFrom, pathTo);
}

/* Копирование */
void* copy_thread(void *data)
{
    struct copyData *copyData;
    copyData = (struct copyData*)data;    
    copy(copyData->pathFrom, copyData->pathTo);
    pthread_exit((void*)0);
}

void* progress_bar(void *data)
{
    struct progressBarData *pbData;
    pbData = (struct progressBarData*)data;
    unsigned long long bytes_to_copy = 0;
    unsigned long long onePercentSize = 0;
    unsigned long long bytes_copied = 0;
    int isDir = 0;
    int result;
    struct stat filestatPathFrom;
    struct stat filestatPathTo;
    cursed_window *win = NULL;
    
    result = lstat(pbData->data.pathFrom, &filestatPathFrom);
    if(result != 0)    
    {
        perror("progress_bar: get file stat");
        pthread_exit((void*)&result);
    }

    /* Объем данных для копирования */
    if(S_ISDIR(filestatPathFrom.st_mode))
    {
        bytes_to_copy = calculate_size(pbData->data.pathFrom);
        isDir = 1;
    }                   
    else
        if(S_ISREG(filestatPathFrom.st_mode))
        {
            bytes_to_copy = filestatPathFrom.st_size;
        }
    onePercentSize = bytes_to_copy/100;
    
    win = create_service_window(pbData->panels, win);

    while(bytes_copied < bytes_to_copy)
    {
        /* if(isDir)
            bytes_done = calculate_size(pbData->data.pathTo);
        else 
        {
            result = lstat(pbData->data.pathTo, &filestatPathTo);
            if(result != 0)
            {
                perror("progress_bar: get file stat");
                pthread_exit((void*)&result);
            }
            bytes_done = filestatPathTo.st_size;
        } */

        pthread_mutex_lock(&shared.mutex);
        bytes_copied = shared.bytes_done;
        pthread_mutex_unlock(&shared.mutex);
        pbData->progress = (bytes_copied/onePercentSize)/100; /* Прогресс в относительной величине (относительно 1) */        
        tui_progress_bar(win->overlay, pbData->progress);
        update_service_panel(win);
        sleep(1);
    }
    tui_del_service_window(win);
    pthread_exit((void*)0);
}

int create_threads(char *pathFrom, char *pathTo, cursed_window **windows)
{
    int result = 0;
	pthread_t tid_copy, tid_pb;
    struct stat filestat;	
    struct copyData data;

    strncpy(data.pathFrom, pathFrom, PATH_MAX);
    strncpy(data.pathTo, pathTo, PATH_MAX);

    struct progressBarData pbData;

    mempcpy(&pbData.data, &data, sizeof(struct copyData));
    pbData.panels = windows;
    
    result = lstat(pathFrom, &filestat);
    if(result == 0)
    {
            result = pthread_create(&tid_copy, NULL, copy_thread, &data);
	        if (result != 0)
            {
		        perror("Creating the copy_thread");
		        return EXIT_FAILURE;
	        }

            result = pthread_create(&tid_pb, NULL, progress_bar, &pbData);
            if (result != 0) 
            {
                perror("Creating the progress_bar thread");
                return EXIT_FAILURE;
            }

            result = pthread_join(tid_copy, NULL);
            if (result != 0) 
            {
                perror("Joining the copy_thread");
                return EXIT_FAILURE;
            }

            result = pthread_join(tid_pb, NULL);
            if (result != 0) 
            {
                perror("Joining the progress_bar thread");
                return EXIT_FAILURE;
            } 
    }
    else
    {
        perror("Create threads");
        exit(errno);
    }
	return EXIT_SUCCESS;
}
