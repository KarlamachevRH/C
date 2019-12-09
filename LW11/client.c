/*   МЕТОДИЧЕСКИЕ УКАЗАНИЯ. 
1. Для отладки приложения рекомендуется запускать клиент и сервер на одной машине.
2. Для отладки рекомендуется использовать отладчик gdb, информацию котором можно получить инструкцией man gdb.
3. Выбор протокола передачи данных определяется вариантом задания.
4. Для нормального выполнения сетевых соединений между компьютерами требуется соответствующая настройка межсетевых экранов.
	ПОРЯДОК ВЫПОЛНЕНИЯ РАБОТЫ. 
1. Написать и отладить функцию, реализующую сервер, получающий и обрабатывающий запросы от клиентов (аналог родительского приложения).
2. Написать и отладить программу, реализующую клиентский процесс (аналог дочернего процесса), запрашивающий у сервера исходные данные, выполняющий вычисления (действия) в соответствии с вариантом, и возвращающий серверу результаты вычислений.
	ВАРИАНТЫ ЗАДАНИЙ. 
См. варианты заданий в файле variants.txt.
Для нечетных вариантов используется протокол TCP, для четных – UDP.
    ВАРИАНТ ЗАДАНИЯ.
10. Авиаразведка. Создается условная карта в виде матрицы, размерность которой определяет размер карты, содержащей произвольное количество единиц (целей) в произвольных ячейках. Из произвольной точки карты стартуют несколько разведчиков (процессов), курсы которых выбираются так, чтобы покрыть максимальную площадь карты. Каждый разведчик фиксирует цели, чьи координаты совпадают с его координатами и по достижении границ карты сообщает количество обнаруженных целей. */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>  

#define h_addr  h_addr_list[0]
#define SCOUTS_NUM 6 //количество разведчиков

//используем семафоры для доступа процессов к сокету
union semun {
 int val;                  // значение для SETVAL 
 struct semid_ds *buf;     // буферы для  IPC_STAT, IPC_SET 
 unsigned short *array;    // массивы для GETALL, SETALL 
                           // часть, особенная для Linux: 
 struct seminfo *__buf;    // буфер для IPC_INFO 
};  

//координаты начала поиска
struct mapPoint {
	int x;
	int y;
};

void error(const char *msg); //вывод сообщения об ошибке
int send_all_data(int sock, void *buff, int len); // отправить весь буфер целиком
int recieve_all_data(int sock, void *buff, int len); // получить буфер известного размера
void generate_start_point (struct mapPoint *start, int mapX, int mapY); //получение точки старта разведчиков
int** get_map(int my_sock, int *mapX, int *mapY, struct mapPoint *start); // получить карту с целями и точку старта разведчиков
int check_point(int **map, int x, int y); //проверка точки карты на наличие цели
void rec_target_coordinates(int x, int y, struct mapPoint *buff); // запись координат цели в буфер

void seek_targets(int **map, int mapY, int startPointX, int startPointY, 
int partStartX, int partSize , struct mapPoint *buff); //поиск целей на карте и запись в буфер
int map_part(int mapX, int scoutsNum); // выделение части карты для 1 разведчика
int map_remains(int mapX, int scoutsNum); // остатки карты
void seek_in_scouts_part_of_map(int **map, int mapY, int partStartX, int partSize, struct mapPoint *buff); //поиск целей разведчиком в выделенной части карты
void show_targets(struct mapPoint *targets, int numOfTargets);

int main(int argc, char *argv[]) {       
    int my_sock, portno;
    int pid[SCOUTS_NUM]; // id номер потока
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int mapX = 0, mapY = 0; //размеры карты авиаразведки	
    int **map; 
    struct mapPoint start; //точка старта разведчиков на карте
    struct mapPoint *targets; //указатель для буферного массива хранения найденных целей   
	
    printf("TCP DEMO CLIENT\n");
	
    if (argc < 3) {
       fprintf(stderr,"Usage %s <hostname> <port>\n", argv[0]);
       exit(0);
    }  

    // извлечение порта
	portno = atoi(argv[2]);   
    
	// Шаг 1 - создание сокета
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) 
        error("ERROR opening socket");
    // извлечение хоста
	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // заполнение структуры serv_addr
    memset(&serv_addr, 0, sizeof(serv_addr));	
    serv_addr.sin_family = AF_INET;
    memmove(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    // установка порта
	serv_addr.sin_port = htons(portno);
    
     // Шаг 2 - установка соединения
    if (connect(my_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
        exit(EXIT_FAILURE);
    }
    
    map = get_map(my_sock, &mapX, &mapY, &start); //получение карты с целями и стартовой точкой для разведчиков
    printf("Карта получена\n");
        
    /* Освободим сокет */
    close(my_sock);
    printf("mapX: %d, mapY: %d\n", mapX, mapY);
    printf("Точка старта: x:%d y:%d\n", start.x, start.y);

    int targetsBuffSize = (mapX*mapY)*sizeof(struct mapPoint);
    targets = (struct mapPoint*)calloc(targetsBuffSize, sizeof(struct mapPoint)); //memory allocation for targets buffer

    int clientsNum = 0;
    for (int i = 0; i < SCOUTS_NUM; i++){ 
        sleep(1);      	
        pid[i] = fork();   
        if (pid[i] < 0)
            error("ERROR on fork");
        if (pid[i] == 0) { 
            //ищем цели на карте
            int partSize = map_part(mapX, SCOUTS_NUM);
            int mapRemains = map_remains(mapX, SCOUTS_NUM);            
            int firstIndex = clientsNum*partSize; 

            seek_targets(map, mapY, start.x, start.y, firstIndex, partSize, targets);

            if(mapRemains && clientsNum == (SCOUTS_NUM-1)) {
                int j = 0;
                while(targets[j].x != 0 || targets[j].y != 0) j++;
                seek_in_scouts_part_of_map(map, mapY, firstIndex+partSize, mapRemains, &targets[j]); 
            } 
            
            int j = 0;            
            while(targets[j].x != 0 || targets[j].y != 0) j++;  
            j++;          
            struct mapPoint *targetsBuff;
            //для отправки количества целей используем буфер по размеру данных
            targetsBuff = (struct mapPoint*)calloc(j, sizeof(struct mapPoint)); 
            //инициализация промежуточного буфера для отправки результатов поиска целей серверу
            for(int i = 0; i < j; i++)
                targetsBuff[i] = targets[i]; 
            printf("Количество найденных целей разведчика №%d: %d\n", ++clientsNum, --j);
            show_targets(targetsBuff, j);

            int dataSizeSend = j*sizeof(struct mapPoint);
            // Шаг 3 - чтение и передача сообщений
            my_sock = socket(AF_INET, SOCK_STREAM, 0);                            
            if (connect(my_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
                error("ERROR connecting");
                exit(EXIT_FAILURE);
            }
            
            send_all_data(my_sock, &dataSizeSend, sizeof(int)); //отправка данных
            send_all_data(my_sock, targetsBuff, dataSizeSend);
            
            // Освободим сокет 
            close(my_sock);                
                        
            printf("Targets sended by scout #%d\n", clientsNum);
            if(targetsBuff != NULL) free(targetsBuff);
            exit(EXIT_SUCCESS);
        }
        clientsNum++;       
    }    
    for(int i = 0; i < mapX; i++){
        if(map[i] != NULL)free(map[i]);
    }
    if(map != NULL)free(map);
    if(targets != NULL)free(targets);    
    exit(EXIT_SUCCESS);	
}

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// отправить весь буфер целиком
int send_all_data(int sock, void *buff, int len) {
    int total = 0;
    int n = 0;

    while(total < len && n >= 0) {
        n = send(sock, buff+total, len-total, 0);        
        total += n;
    }
    return (n==-1 ? -1 : total);
}

// получить буфер известного размера
int recieve_all_data(int sock, void *buff, int len){
	int total = 0;
    int n = 1;

    while(total < len && n > 0) {		
        n = recv(sock, buff+total, len-total, 0);        
        total += n;
    }	
    return (n == 0 ? 0 : total);
}

//получение точки старта разведчиков
void generate_start_point (struct mapPoint *start, int mapX, int mapY){	
    srand ( time(NULL) + 60);
    start->x = rand() % mapX;
    start->y = rand() % mapY;		
}

int** get_map(int my_sock, int *mapX, int *mapY, struct mapPoint *start){    
	int bytes_recv = 0; // размер принятого сообщения		
	int dataSizeRecv = 0; // размер получаемых данных    
    int mapSizeBuff[2] = {0}; // буфер для получения размерности карты
    
    //recieve map size
    bytes_recv =  recieve_all_data(my_sock, mapSizeBuff, sizeof(mapSizeBuff));
    if (bytes_recv <= 0) error("ERROR reading from socket");

    *mapX = mapSizeBuff[0], *mapY = mapSizeBuff[1];
    printf("Client map: mapX:%d, mapY:%d \n", *mapX, *mapY);
    int **map = (int**)calloc(*mapX, sizeof(int*)); //карта авиаразведки
	if (map==NULL) {
		printf("Memory allocation error!\n");
		exit(EXIT_FAILURE);
	}
	for(int i = 0; i<(*mapX); i++){
		map[i] = (int*)calloc(*mapY, sizeof(int));
		if (map[i]==NULL) {
			printf("Memory allocation error!\n");
            free(map);
			exit(EXIT_FAILURE);
		}
	}
    //recieve data size
    bytes_recv =  recieve_all_data(my_sock, &dataSizeRecv, sizeof(dataSizeRecv));
    if (bytes_recv <= 0) error("ERROR reading from socket");
    
    for(int i = 0; i < *mapX; i++){
		bytes_recv = recieve_all_data(my_sock, map[i], dataSizeRecv); //получение карты с отмеченными целями
		if (bytes_recv < 0) error("ERROR send data to socket");  
	}

    for(int i = 0; i<*mapX; i++){
        for(int j = 0; j<*mapY; j++) {
            if(map[i][j] == 1) 
            printf("Recieved target: x: %d, y: %d\n", i, j);
        }           
    }
    
    //получаем точку старта разведчиков на карте
    generate_start_point (start, *mapX, *mapY);
    return map;
}

int map_part(int mapX, int scoutsNum){
    return mapX/scoutsNum;
}

int map_remains(int mapX, int scoutsNum){
    return mapX%scoutsNum;
}

void seek_targets(int **map, int mapY, int startPointX, int startPointY, int partStartX, int partSize , struct mapPoint *buff){        
    for(int i = startPointY; i > 0; i--){ //идем вниз по карте по координате y  до y = 0        
        if(check_point(map, startPointX, i)){
            rec_target_coordinates(startPointX, i, buff);
            buff++;
        }         
    }
    if(startPointX > partStartX) { //если точка старта по оси Х справа от начала зоны поиска, то идем налево
        for(int i = startPointX; i > partStartX; i--){ //идем налево по х - координатам до х = partStartX            
            if(check_point(map, i, 0)){
                rec_target_coordinates(i, 0, buff);  
                buff++; 
            }
                          
        }        
    } else{ //если точка старта по оси Х слева от начала зоны поиска, то идем направо
        for(int i = startPointX; i < partStartX; i++){ //идем налево по х - координатам до х = partStartX            
            if(check_point(map, i, 0)){
                rec_target_coordinates(i, 0, buff);  
                buff++;
            }                  
        }        
    }
    seek_in_scouts_part_of_map(map, mapY, partStartX, partSize, buff);       
}

int check_point(int **map, int x, int y){
    int istarget = 0;
    if(map[x][y] == 1) istarget = 1;
    return istarget;
}

void rec_target_coordinates(int x, int y, struct mapPoint *buff){
    buff->x = x;
    buff->y = y;    
}

void seek_in_scouts_part_of_map(int **map, int mapY, int partStartX, int partSize, struct mapPoint *buff){   
    int switcher = 0;
    for(int i = partStartX; i < partStartX+partSize; i++){        
        if(switcher%2 == 0){
            for(int j = 0; j < mapY; j++){                
                if(check_point(map, i, j)){
                    rec_target_coordinates(i, j, buff);
                    buff++;
                }                 
            } 
        }
        else{
            for(int j = mapY-1; j >= 0; j--){                
                if(check_point(map, i, j)){
                    rec_target_coordinates(i, j, buff);
                    buff++;
                }                
            }                         
        }
        switcher++;               
    }    
}

void show_targets(struct mapPoint *targets, int numOfTargets){
    int cnt = 0;	
	for (int i = 0;  i < numOfTargets; i++){
        if(targets[i].x != 0 || targets[i].y != 0)        
		printf("Target #%d: x = %d, y = %d\n", cnt, targets[i].x, targets[i].y);
        cnt++;
	}    
}