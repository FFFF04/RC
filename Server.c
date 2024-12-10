#include "Server.h"
#include "extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>

#define max(A,B) ((A)>=(B)?(A):(B))

int verbose_mode = 0; // 0 desativado; 1 ativado
FILE *fptr;
char colors[6] = {'R', 'G', 'B', 'Y', 'O', 'P'};
long int clock_my = 0;

DIR *DIR_games;
struct dirent *dp_games;
DIR *DIR_scores;
struct dirent *dp_scores;



char* start(char* arguments){

    srand((unsigned int) time(NULL));

    char* PLID, *time, *endptr;
    long int num_PLID, num_time;
    char solution[4];
    FILE *game_file;
    int created = 0;    
    int charcount = 0;

    for(int i = 0; arguments[i]; i++) {
        if(arguments[i] == ' ') 
            charcount++;
    }
    if (charcount != 1)
        return "RSG ERR\n";
    

    PLID = strtok(arguments, " ");
    time = strtok(NULL, " ");

    if (strlen(PLID) != 6)
        return "RSG ERR\n";
    
    for (size_t i = 0; i < strlen(PLID); i++){
        if (isdigit(PLID[i]) == 0)
            return "RSG ERR\n";
    }
    for (size_t i = 0; i < strlen(time)-1; i++){
        if (isdigit(time[i]) == 0 )
            return "RSG ERR\n";
    }
    
    num_PLID = strtol(PLID, &endptr, 10);
    num_time = strtol(time, &endptr, 10);
    if (num_PLID == 0 || num_time == 0 || num_time > 600)
        return "RSG ERR\n";
    

    created = CheckGameFileExists("GAMES", num_PLID);
    if (created == 1)
        return "RSG NOK\n";


    game_file = CreateAndOpenGameFile("GAMES/GAME_", num_PLID, "w+");
    if (game_file == NULL)
        return "RSG ERR\n";

    for (size_t i = 0; i < 4; i++)
        solution[i] = colors[rand() % 6];
    solution[4] = '\0';

    WriteGameStart(game_file, num_PLID, "P", solution, num_time);

    fclose(game_file);
    return "RSG OK\n";
}



void TRY(char* arguments, char *res_msg){

    char* PLID, *endptr, *solution_string, *first_line, *rest_file, *buffer;
    char solution[5], color_code[5], dirpath[256];
    long int num_PLID, start_time, num_nt; 
    int duration, difference, created, *nB, *nW, charcount = 0;
    int  index = 0;
    size_t i, file_size;
    FILE *game_file;
    time_t raw_time;
    DIR* DIR_player_games;

    for(i = 0; arguments[i]; i++) {
        if(arguments[i] == ' ') 
            charcount++;
    }
    if (charcount != 5){ 
        strcat(res_msg,"RTR ERR\n");
        return;    
    }
    
    PLID = strtok(arguments, " ");
    if (strlen(PLID) != 6){ 
        strcat(res_msg,"RTR ERR\n");
        return;    
    }
    
    for (i = 0; i < strlen(PLID); i++){
        if (isdigit(PLID[i]) == 0){ 
            strcat(res_msg,"RTR ERR\n");
            return;    
        }
    }
    
    num_PLID = strtol(PLID, &endptr, 10);
    if (num_PLID == 0){ 
        strcat(res_msg,"RTR ERR\n");
        return;    
    }
    solution_string = strtok(NULL, " ");
    i = 0;
    while (i <= 3) {
        char color = solution_string[0];
        int valid = 0;

        for (int j = 0; j < 6; j++) {
            if (toupper(color) == colors[j]) {
                valid = 1;
                break;
            }
        }
        if (!valid){ 
            strcat(res_msg,"RTR ERR\n");
            return;    
        }   
        if (index < 4) 
            color_code[index++] = toupper(color);
        
        solution_string = strtok(NULL, " ");
        i++;
    }
    color_code[index] = '\0';

    num_nt = strtol(solution_string, &endptr, 10);
    
    created = CheckGameFileExists("GAMES", num_PLID);
    if (created == 0){
        strcat(res_msg,"RTR NOK\n");
        return;
    }
    
    game_file = CreateAndOpenGameFile("GAMES/GAME_", num_PLID, "r+");
    if (game_file == NULL){
        strcat(res_msg,"RTR ERR\n");
        return;
    }

    fseek(game_file, 0, SEEK_END);
    file_size = ftell(game_file);
    rewind(game_file);

    buffer = (char *)malloc(file_size + 1);
    if (fread(buffer, 1, file_size, game_file) != file_size) {
        perror("fread");
        free(buffer);
        strcat(res_msg,"RTR ERR\n");
        return;
    }
    buffer[file_size] = '\0';
    first_line = strtok(buffer,"\n");
    rest_file = strtok(NULL,"");
    time(&raw_time);
    sscanf(first_line, "%*6s %*1c %4s %d %*10s %*8s %ld", solution, &duration, &start_time);

    difference = raw_time - start_time;
    if(duration <= difference){ // JÁ PASSOU DO TEMPO DE JOGO

        DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
        if (DIR_player_games == NULL){
            strcat(res_msg,"RTR ERR\n");
            return;
        }
        snprintf(dirpath, sizeof(dirpath), "GAMES/%ld", num_PLID);
        CreateTimestampedFile_TRY(dirpath, first_line, rest_file, 'T', localtime(&raw_time), duration);
        closedir(DIR_player_games);
        sprintf(res_msg, "RTR ETM %s\n", solution);
        removeFile(game_file,"GAMES/GAME_",num_PLID);
        return;
    }
    printf("num_nt:%ld\nsolution:%s\ncolor:%s\n",num_nt, solution,color_code);
    if(num_nt == 8 && strcmp(solution,color_code) != 0){ // JÁ NÃO HA MAIS JOGADAS

        DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
        if (DIR_player_games == NULL){
            strcat(res_msg,"RTR ERR\n");
            return;
        }
        snprintf(dirpath, sizeof(dirpath), "GAMES/%ld", num_PLID);
        CreateTimestampedFile_TRY(dirpath, first_line, rest_file, 'F', localtime(&raw_time), difference);
        closedir(DIR_player_games);
        sprintf(res_msg, "RTR ENT %s\n", solution);
        removeFile(game_file,"GAMES/GAME_",num_PLID);
        return;
    }
    else if (strcmp(solution,color_code) == 0){
        sprintf(res_msg, "RTR OK %ld %d %d\n", num_nt, 4, 0);
    }

        nB = 0;
        nW = 0;

        //calculate_blacks_and_whites(color_code, solution, nB, nW);
        sprintf(res_msg, "RTR OK %ld %ls %ls\n", num_nt, nB, nW);

    return;
}



void quit(char* arguments, char *res_msg){ //UDP protocol

    char *endptr, *buffer, *first_line, *rest_file, *res_function_msg;
    int num_PLID, created;
    char dirpath[256];
    DIR *DIR_player_games;
    FILE *game_file;
    size_t file_size;

    int charcount = 0;

    for(int i = 0; arguments[i]; i++) {
        if(arguments[i] == ' ')
            charcount++;
    }
    if (charcount != 0){
        strcat(res_msg,"RQT ERR\n");
        return;
    }

    num_PLID = strtol(arguments, &endptr, 10);
    created = CheckGameFileExists("GAMES", num_PLID);
    if (created == 0){
        strcat(res_msg,"RQT NOK\n");
        return;
    }
    game_file = CreateAndOpenGameFile("GAMES/GAME_", num_PLID, "r+");
    if (game_file == NULL){
        strcat(res_msg,"RQT NOK\n");
        return;
    }

    fseek(game_file, 0, SEEK_END);
    file_size = ftell(game_file);
    rewind(game_file);

    buffer = (char *)malloc(file_size + 1);
    if (fread(buffer, 1, file_size, game_file) != file_size) {
        perror("fread");
        free(buffer);
        strcat(res_msg,"RQT ERR\n");
        return;
    }
    buffer[file_size] = '\0';
    first_line = strtok(buffer,"\n");
    rest_file = strtok(NULL,"");

    snprintf(dirpath, sizeof(dirpath), "GAMES/%d", num_PLID);
    
    DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
    if (DIR_player_games == NULL){
        strcat(res_msg,"RQT ERR\n");
        return;
    }
    
    res_function_msg = (char*) calloc(20,1);
    CreateTimestampedFile_QUIT(dirpath,first_line, rest_file, res_function_msg);
    sprintf(res_msg, "RQT %s\n", res_function_msg);

    closedir(DIR_player_games);
    removeFile(game_file,"GAMES/GAME_",num_PLID);
    free(res_function_msg);
    return;
}



char* debug(char* arguments){

    char *PLID, *time, *endptr, *solution_string, *solution_string_less_n;
    char solution[4];
    long int num_PLID, num_time;
    FILE *game_file;
    int created = 0;    
    int charcount = 0;
    int index = 0;

    for(int i = 0; arguments[i]; i++) {
        if(arguments[i] == ' ')
            charcount++;
    }
    if (charcount != 5)
        return "RDB ERR\n";

    PLID = strtok(arguments, " ");
    time = strtok(NULL, " ");
    if (strlen(PLID) != 6)
        return "RDB ERR\n";
    
    for (size_t i = 0; i < strlen(PLID); i++){
        if (isdigit(PLID[i]) == 0)
            return "RDB ERR\n";
    }
    for (size_t i = 0; i < strlen(time); i++){
        if (isdigit(time[i]) == 0)
            return "RDB ERR\n";
    }
    
    num_PLID = strtol(PLID, &endptr, 10);
    num_time = strtol(time, &endptr, 10);
    if (num_PLID == 0 || num_time == 0 || num_time > 600)
        return "RDB ERR\n";

    solution_string = strtok(NULL, "\n");
    solution_string_less_n = strtok(solution_string, " ");
    while (solution_string_less_n != NULL) {

        char color = solution_string_less_n[0];
        int valid = 0;

        for (int i = 0; i < 6; i++) {
            if (toupper(color) == colors[i]) {
                valid = 1;
                break;
            }
        }
        if (!valid) 
            return "RDB ERR\n";        
        if (index < 4) 
            solution[index++] = toupper(color);
        
        solution_string_less_n = strtok(NULL, " ");
    }
    solution[index] = '\0';
    created = CheckGameFileExists("GAMES", num_PLID);
    if (created == 1)
        return "RDB NOK\n";

    game_file = CreateAndOpenGameFile("GAMES/GAME_", num_PLID, "w+");
    if (game_file == NULL)
        return "RDB ERR\n";

    WriteGameStart(game_file, num_PLID, "D", solution, num_time);

    fclose(game_file);
    return "RDB OK\n";
}



int main(int argc, char *argv[]){
    char *port, *command, *arguments;
    struct sockaddr_in addr;
    socklen_t addrlen;
    struct addrinfo hints_udp, hints_tcp,*res_tcp, *res_udp;
    struct sigaction act;
    int fd_tcp, fd_udp, errcode, select_fds;
    
    fd_set inputs, testfds;
    //enum {idle,busy} state;
    char buffer[128];

    if (argc <= 0 || argc > 4){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1)        
        port = "58014";// Port 58000 + nº Grupo(14)
        
    else if (argc == 2){
        if (strcmp(argv[1],"-p") == 0)
            port = argv[2];

        else{
            port = "58014";     // Port 58000 + nº Grupo(14)
            verbose_mode = 1;
        }
    }
    else{
        port = argv[2];
        verbose_mode = 1;
    }

    const char* dir_games = "GAMES";
    struct stat sb_games;
    const char* dir_scores = "SCORES";
    struct stat sb_scores;
 
    if (stat(dir_games, &sb_games) == -1) //Path does not exist
        mkdir(dir_games, 0777);
    
    if (stat(dir_scores, &sb_scores) == -1) //Path does not exist
        mkdir(dir_scores, 0777);
    
    if ((DIR_games = opendir(dir_games)) == NULL) {
        perror ("Cannot open dir GAMES");
        exit(EXIT_FAILURE);
    }
    if ((DIR_scores = opendir(dir_scores)) == NULL) {
        perror ("Cannot open dir SCORES");
        exit(EXIT_FAILURE);
    }

    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE,&act,NULL) == -1)/*error*/
        exit(EXIT_FAILURE);



    //PARA UDP
    fd_udp=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
    if(fd_udp == -1)/*error*/
        exit(EXIT_FAILURE);

    memset(&hints_udp,0,sizeof hints_udp);
    hints_udp.ai_family= AF_UNSPEC;//IPv4
    hints_udp.ai_socktype=SOCK_DGRAM;//UDP socket
    hints_udp.ai_flags = AI_PASSIVE;

    errcode=getaddrinfo(NULL,port,&hints_udp,&res_udp);
    if(errcode != 0)/*error*/
        exit(EXIT_FAILURE);
    
    if(bind(fd_udp,res_udp->ai_addr, res_udp->ai_addrlen) == -1)/*error*/
        exit(EXIT_FAILURE);
    


    //PARA TCP
    fd_tcp=socket(AF_INET,SOCK_STREAM,0);//TCP socket
    if(fd_tcp==-1)
        exit(EXIT_FAILURE);
    memset(&hints_tcp,0,sizeof hints_tcp);
    hints_tcp.ai_family=AF_INET;//IPv4
    hints_tcp.ai_socktype=SOCK_STREAM;//TCP socket
    hints_tcp.ai_flags = AI_PASSIVE;
    errcode=getaddrinfo(NULL, port, &hints_tcp, &res_tcp);
    if(errcode != 0)/*error*/
        exit(EXIT_FAILURE);
    if(bind(fd_tcp, res_tcp->ai_addr, res_tcp->ai_addrlen) == -1)/*error*/{
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    if(listen(fd_tcp,5) == -1)/*error*/ 
        exit(EXIT_FAILURE);

    FD_ZERO(&inputs); // Clear input mask
    FD_SET(fd_udp,&inputs); // Set UDP channel on
    FD_SET(fd_tcp,&inputs); // Set TCP channel on

    while(1){
        testfds = inputs; // Reload mask

        select_fds = select(FD_SETSIZE,&testfds,(fd_set *)NULL,(fd_set *)NULL,(struct timeval *) NULL);

        switch(select_fds){
            case -1:
                perror("select");
                exit(1);
            default:
                if(FD_ISSET(fd_tcp,&testfds)){
                    //printf("ola TCP");
                }
                if(FD_ISSET(fd_udp,&testfds)){

                    addrlen=sizeof(addr);
                    if(recvfrom(fd_udp,buffer,128,0,(struct sockaddr*)&addr,&addrlen) == -1)/*error*/
                        exit(EXIT_FAILURE);
                    
                    command = strtok(buffer, " ");
                    arguments = strtok(NULL, "");
            
                    if (strcmp(command,"SNG") == 0){
                        char* res_msg = start(arguments);
                        if(sendto(fd_udp, res_msg, strlen(res_msg), 0, (struct sockaddr*)&addr, addrlen) == -1)/*error*/
                            exit(EXIT_FAILURE);
                    }
                    else if (strcmp(command,"TRY") == 0){
                        char *res_msg = (char*) calloc(30,1);
                        TRY(arguments,res_msg);
                        if(sendto(fd_udp, res_msg, strlen(res_msg), 0, (struct sockaddr*)&addr, addrlen) == -1)/*error*/
                            exit(EXIT_FAILURE);
                        free(res_msg);
                    }
                    else if (strcmp(command,"QUT") == 0){
                        char *res_msg = (char*) calloc(20,1);
                        quit(arguments,res_msg);
                        if(sendto(fd_udp, res_msg, strlen(res_msg), 0, (struct sockaddr*)&addr, addrlen) == -1)/*error*/
                            exit(EXIT_FAILURE);
                        free(res_msg);
                    }
                    else if (strcmp(command,"DBG") == 0){
                        char* res_msg = debug(arguments);
                        if(sendto(fd_udp, res_msg, strlen(res_msg), 0, (struct sockaddr*)&addr, addrlen) == -1)/*error*/
                            exit(EXIT_FAILURE);
                    }
                    memset(buffer, 0, sizeof(buffer));
                }
        }
    }
    
    freeaddrinfo(res_tcp);
    freeaddrinfo(res_udp);
    close(fd_udp);
    close(fd_tcp);

    exit(EXIT_SUCCESS);
}