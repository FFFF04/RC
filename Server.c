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

    char trash[5], PLID[7], solution[4];
    char *endptr;
    long int num_PLID, num_time;
    FILE *game_file;
    int created = 0;    
    int charcount = 0;

    for (int i = 0; arguments[i]; i++) {
        if (arguments[i] == ' ')
            charcount++;
    }

    if (charcount != 1)
        return "RSG ERR\n";

    if (sscanf(arguments, "%6s %ld%s", PLID, &num_time, trash) != 2)
        return "RSG ERR\n";
    
    if (strlen(PLID) != 6 || strspn(PLID, "0123456789") != 6 || num_time <= 0 || num_time > 600)
        return "RSG ERR\n";

    num_PLID = strtol(PLID, &endptr, 10);

    created = CheckGameFileExists("GAMES", num_PLID, 1);

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

    char*first_line, *rest_file, *buffer, *tries, *all_tries, *line_try;
    char mode, solution[5], color_code[5], dirpath[256], repeted_code[5];
    long int num_PLID, start_time, num_nt; 
    int duration, difference, created, num_tries = 0, charcount = 0, nB = 0, nW = 0;
    size_t file_size;
    FILE *game_file;
    time_t raw_time;
    DIR* DIR_player_games;

    for(size_t i = 0; arguments[i]; i++) {
        if(arguments[i] == ' ') 
            charcount++;
    }
    if (charcount != 5){ 
        strcat(res_msg,"RTR ERR\n");
        return;    
    }
    
    if (sscanf(arguments, "%6ld %c %c %c %c %ld", &num_PLID, &color_code[0], &color_code[1], 
            &color_code[2], &color_code[3], &num_nt) != 6){
        strcat(res_msg,"RTR ERR\n");
        return;
    }
    color_code[4] = '\0';

    for (size_t i = 0; i != 4; i++) {
        if (color_code[i] != ' ') {
            int valid = 0;
            for (size_t j = 0; j < 6; j++) {
                if (color_code[i] == colors[j]) {
                    valid = 1;
                    break;
                }
            }
            if (valid == 0){
                strcat(res_msg,"RTR ERR\n");
                return;
            }
        }
    }
    
    created = CheckGameFileExists("GAMES", num_PLID, 0);
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
    tries = (char *) calloc(256,1);
    if (rest_file != NULL)
        memcpy(tries, rest_file, 256);
    

    time(&raw_time);
    sscanf(first_line, "%*6s %1c %4s %d %*10s %*8s %ld", &mode, solution, &duration, &start_time);


    difference = raw_time - start_time;
    if(duration <= difference){ // JÁ PASSOU DO TEMPO DE JOGO

        DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
        if (DIR_player_games == NULL){
            free(buffer);
            free(tries);
            strcat(res_msg,"RTR ERR\n");
            return;
        }
        snprintf(dirpath, sizeof(dirpath), "GAMES/%ld", num_PLID);
        CreateTimestampedFile_TRY(dirpath, first_line, rest_file, 'T', localtime(&raw_time), duration);
        free(buffer);
        free(tries);
        closedir(DIR_player_games);
        sprintf(res_msg, "RTR ETM %s\n", solution);
        removeFile(game_file,"GAMES/GAME_",num_PLID);
        return;
    }


    if(num_nt == 8 && strcmp(solution,color_code) != 0){ // JÁ NÃO HA MAIS JOGADAS

        DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
        if (DIR_player_games == NULL){
            strcat(res_msg,"RTR ERR\n");
            free(buffer);
            free(tries);
            return;
        }
        snprintf(dirpath, sizeof(dirpath), "GAMES/%ld", num_PLID);
        CreateTimestampedFile_TRY(dirpath, first_line, rest_file, 'F', localtime(&raw_time), difference);
        free(buffer);
        free(tries);
        closedir(DIR_player_games);
        sprintf(res_msg, "RTR ENT %s\n", solution);
        removeFile(game_file,"GAMES/GAME_",num_PLID);
        return;
    }

    line_try = strtok(tries, "\n");
    while (line_try != NULL){ // CONFIRMAR SE NÃO HÁ JOGADA REPETIDA
        
        sscanf(line_try, "T: %4s %d %d", repeted_code, &nB, &nW);

        if (strcmp(repeted_code, color_code) == 0 && num_nt != num_tries){
            free(tries);
            free(buffer);
            fclose(game_file);
            sprintf(res_msg, "RTR DUP\n");
            return;
        }
        if (num_nt == num_tries){
            num_tries = -1;
            if (strcmp(repeted_code, color_code) != 0){
                free(tries);
                free(buffer);
                fclose(game_file);
                sprintf(res_msg, "RTR INV\n");
                return;
            }
            sprintf(res_msg, "RTR OK %ld %d %d\n", num_nt, nB, nW);
            fclose(game_file);
            free(buffer);
            return;
        }
        
        line_try = strtok(NULL, "\n");
        num_tries++;
    }
    free(tries);

    calculate_blacks_and_whites(color_code, solution, &nB, &nW);
    fprintf(game_file,"T: %s %d %d %d\n", color_code, nB, nW, difference); // GUARDA NO FILE
    fflush(game_file);
    
    if (strcmp(solution,color_code) == 0){ // WE WON

        DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
        if (DIR_player_games == NULL){
            strcat(res_msg,"RTR ERR\n");
                free(buffer);
            return;
        }
        snprintf(dirpath, sizeof(dirpath), "GAMES/%ld", num_PLID);
        all_tries = (char *) calloc(256,1);
        if (rest_file != NULL)
            sprintf(all_tries, "%sT: %s %d %d %d\n", rest_file, color_code, nB, nW, difference);
        else
            sprintf(all_tries, "T: %s %d %d %d\n", color_code, nB, nW, difference);

        CreateTimestampedFile_TRY(dirpath, first_line, all_tries, 'W', localtime(&raw_time), difference);
        free(buffer);
        free(all_tries);
        closedir(DIR_player_games);
        removeFile(game_file,"GAMES/GAME_", num_PLID);
        
        // CALCULA O SCORE E GUARDA NA DIRETORIA SCORES
        int score = CalculateScore(num_nt,difference,duration);
        if (CreateFile_SCORE(num_PLID, score, localtime(&raw_time), color_code, num_nt, mode) == 1){
            sprintf(res_msg, "RTR ERR\n");
            return;
        }
        
        sprintf(res_msg, "RTR OK %ld %d %d\n", num_nt, nB, nW);
        return; 
    }

    else{ // NORMAL PLAY
        sprintf(res_msg, "RTR OK %ld %d %d\n", num_nt, nB, nW);
        fclose(game_file);
        free(buffer);
        return; 
    }
}



void show_trials(char *arguments, char *res_msg){
    
    char *endptr, *buffer, *first_line, *rest_file, *line;
    char filename[24], color_code[5], Fdata[2049], protocol_msg[9], last_line[64];
    int num_PLID, created, nW, nB, duration, start_time, difference;
    size_t file_size;
    FILE* game_file;
    time_t raw_time;
    memset(Fdata, 0, sizeof(Fdata));
    memset(protocol_msg, 0, sizeof(protocol_msg));
    num_PLID = strtol(arguments, &endptr, 10);

    created = CheckGameFileExists("GAMES", num_PLID, 1);
    if (created == 0){ // Not created
        if (FindLastGame(num_PLID, filename) == 0){
            strcat(res_msg,"RST NOK\n");
            return;
        }
        game_file = fopen(filename, "r+");
        if (game_file == NULL) {
            strcat(res_msg,"RST NOK\n");
            return;
        }
        strcat(protocol_msg, "RST FIN");
        strcat(Fdata,"Plays of last Game:\n");
    }
    else{// Created
        game_file = CreateAndOpenGameFile("GAMES/GAME_", num_PLID, "r+");
        if (game_file == NULL){
            strcat(res_msg,"RST NOK\n");
            return;
        }
        strcat(protocol_msg, "RST ACT");
        strcat(Fdata,"Previous plays:\n");
    }
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "STATE_%d.txt",num_PLID);

    fseek(game_file, 0, SEEK_END);
    file_size = ftell(game_file);
    rewind(game_file);

    buffer = (char *)malloc(file_size + 1);
    if (fread(buffer, 1, file_size, game_file) != file_size) {
        perror("fread");
        free(buffer);
        strcat(res_msg,"RST NOK\n");
        return;
    }
    buffer[file_size] = '\0';
    first_line = strtok(buffer,"\n");
    rest_file = strtok(NULL,"");
    time(&raw_time);
    
    sscanf(first_line, "%*6s %*1c %*4s %d %*10s %*8s %d", &duration, &start_time);
    difference = raw_time - start_time;
    if(duration <= difference)
        difference = duration;
    
    if (rest_file == NULL){
        strcat(Fdata, "-- No transactions found --\n");
        sprintf(last_line,"-- Time left: %d --\n", duration - difference);
    }
        
    else{
        line = strtok(rest_file,"\n");
        if(strcmp(protocol_msg, "RST FIN") == 0 && line != NULL && line[0] != 'T')
            strcat(Fdata, "-- No transactions found --\n");
        
        if (strcmp(protocol_msg, "RST FIN") == 0)
            sprintf(last_line,"-- Duration of Last Game: %d --\n", difference);
        else
            sprintf(last_line,"-- Time left: %d --\n", duration - difference);
        
        while (line != NULL && line[0] == 'T'){
            char res_line[23];
            sscanf(line, "%*2s %4s %d %d", color_code, &nB, &nW);
            sprintf(res_line, "T: %s, nB: %d, nW: %d\n", color_code, nB, nW);
            strcat(Fdata,res_line);
            line = strtok(NULL,"\n");
        }
    }

    sprintf(res_msg, "%s %s %d %s%s",protocol_msg, filename, calculate_file_size(Fdata,last_line), Fdata, last_line);
    free(buffer);
}



void scoreboard(char *res_msg){
    SCORELIST *List = (SCORELIST*) malloc(sizeof(SCORELIST));
    time_t raw_time;
    char line_score[128], Fdata[1500], filename[50];

    int n_scores = FindTopScores(List);
    if (n_scores == 0){
        strcat(res_msg,"RSS EMPTY\n");
        free(List);
        return;
    }
    
    time(&raw_time);
    sprintf(Fdata, "--------------TOP 10 SCORES--------------\n   SCORE   PLAYER   CODE   N_TRIALS   MODE\n");

    for (int i = 0; i < n_scores; i++){
        if (List->mode[i] == 1)
            sprintf(line_score,"%d - %d    %s   %s       %d      PLAY\n", 
                i + 1, List->score[i], List->PLID[i], List->color_code[i], List->ntries[i]);
        
        else
            sprintf(line_score,"%d - %d    %s   %s       %d      DEBUG\n", 
                i + 1, List->score[i], List->PLID[i], List->color_code[i], List->ntries[i]);
            
        strcat(Fdata,line_score);

        memset(line_score,0, sizeof(line_score));
    }
    
    sprintf(filename, "TOPSCORES_%ld.txt",raw_time);

    sprintf(res_msg,"RSS OK %s %d %s\n", filename, calculate_file_size(Fdata, ""), Fdata);

    free(List);
}



void quit(char* arguments, char *res_msg){ //UDP protocol
    time_t raw_time;
    long int start_time;
    char *endptr, *buffer, *first_line, *rest_file, *res_function_msg;
    int num_PLID, created, duration, difference, game_end = 0;
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
    created = CheckGameFileExists("GAMES", num_PLID, 0);
    if (created == 0){
        strcat(res_msg,"RQT NOK\n");
        return;
    }
    game_file = CreateAndOpenGameFile("GAMES/GAME_", num_PLID, "r+");
    if (game_file == NULL){
        strcat(res_msg,"RQT ERR\n");
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
    time(&raw_time);

    sscanf(first_line, "%*6s %*1c %*4s %d %*10s %*8s %ld", &duration, &start_time);

    difference = raw_time - start_time;
    if(duration <= difference){ 
        strcat(res_msg, "RQT NOK\n");
        game_end = 1;
    }

    snprintf(dirpath, sizeof(dirpath), "GAMES/%d", num_PLID);
    
    DIR_player_games = SearchAndCreateGameDir("GAMES/", num_PLID);
    if (DIR_player_games == NULL){
        strcat(res_msg,"RQT ERR\n");
        free(buffer);
        return;
    }
    
    res_function_msg = (char*) calloc(20,1);
    CreateTimestampedFile_QUIT(dirpath,first_line, rest_file, res_function_msg);
    if(game_end == 0)
        sprintf(res_msg, "RQT %s\n", res_function_msg);

    closedir(DIR_player_games);
    removeFile(game_file,"GAMES/GAME_",num_PLID);
    free(res_function_msg);
    free(buffer);
    return;
}



char* debug(char* arguments){

    char *endptr;
    char solution[5], PLID[7];
    long int num_PLID, num_time;
    FILE *game_file;
    int created = 0;    
    int charcount = 0;

    for(int i = 0; arguments[i]; i++) {
        if(arguments[i] == ' ')
            charcount++;
    }
    if (charcount != 5)
        return "RDB ERR\n";

    if (sscanf(arguments, "%6s %ld %c %c %c %c", PLID, &num_time, &solution[0], &solution[1], 
            &solution[2], &solution[3]) != 6)
        return "RDB ERR\n";
    solution[4] = '\0';

    if (strlen(PLID) != 6 || strspn(PLID, "0123456789") != 6 || num_time <= 0 || num_time > 600)
        return "RDB ERR\n";

    num_PLID = strtol(PLID, &endptr, 10);

    for (size_t i = 0; i != 4; i++) {
        if (solution[i] != ' ') {
            int valid = 0;
            for (size_t j = 0; j < 6; j++) {
                if (solution[i] == colors[j]) {
                    valid = 1;
                    break;
                }
            }
            if (valid == 0)
                return "RDB ERR\n";
        }
    }

    created = CheckGameFileExists("GAMES", num_PLID,1);
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
    
    fd_set testfds;

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
    fd_tcp=socket(AF_INET,SOCK_STREAM,0);   //TCP socket
    if(fd_tcp==-1)
        exit(EXIT_FAILURE);
    memset(&hints_tcp,0,sizeof hints_tcp);
    hints_tcp.ai_family=AF_INET;    //IPv4
    hints_tcp.ai_socktype=SOCK_STREAM;  //TCP socket
    hints_tcp.ai_flags = AI_PASSIVE;
    errcode=getaddrinfo(NULL, port, &hints_tcp, &res_tcp);
    if(errcode != 0)/*error*/
        exit(EXIT_FAILURE);
    int true = 1;
    setsockopt(fd_tcp,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
    if(bind(fd_tcp, res_tcp->ai_addr, res_tcp->ai_addrlen) == -1){  /*error*/
        perror("bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res_tcp);
    freeaddrinfo(res_udp);
    if(listen(fd_tcp,5) == -1)/*error*/ 
        exit(EXIT_FAILURE);

    FD_ZERO(&testfds); // Clear input mask

    while(1){

        FD_SET(fd_udp,&testfds); // Set UDP channel on
        FD_SET(fd_tcp,&testfds); // Set TCP channel on

        select_fds = select(FD_SETSIZE,&testfds,(fd_set *)NULL,(fd_set *)NULL,(struct timeval *) NULL);

        switch(select_fds){
            case -1:
                perror("select");
                exit(1);
            default:
                if(FD_ISSET(fd_tcp,&testfds)){
                    pid_t p;
                    int newfd, n_read, n_write;
                    p = fork();
                    
                    if(p < 0){
                        perror("fork fail");
                        exit(EXIT_FAILURE);
                    }
                    else if (p == 0){ // Child
                        
                        char *buffer = (char*) calloc(128, 1);
                        char PLID[7];

                        addrlen = sizeof(addr);

                        if((newfd = accept(fd_tcp,(struct sockaddr*)&addr,&addrlen)) == -1)
                            exit(EXIT_FAILURE);/*error*/
                        
                        close(fd_tcp);
                        close(fd_udp);

                        n_read = read(newfd, buffer, 32);
                        if (n_read <= 0) {
                            perror("Read failed");
                            exit(EXIT_FAILURE);
                        }

                        command = strtok(buffer, " ");
                        arguments = strtok(NULL, "");

                        

                        char *res_msg = (char*) calloc(2049,1);
                        if (strcmp(command,"STR") == 0){
                            show_trials(arguments, res_msg);
                            if (verbose_mode == 1){
                                sscanf(arguments,"%6s", PLID);
                                printf("TCP -> PLID:%s, Request:%s, IP:%s, PORT:%d\n", PLID, 
                                    command, (char*)inet_ntoa((struct in_addr)addr.sin_addr), addr.sin_port);
                            }
                        }
                        
                        if (strcmp(command,"SSB\n") == 0){
                            scoreboard(res_msg);           
                            if (verbose_mode == 1)
                                printf("TCP -> Request:%s, IP:%s, PORT:%d\n", 
                                    command, (char*)inet_ntoa((struct in_addr)addr.sin_addr), addr.sin_port);
                        }           
                        
                        char *ptr = &res_msg[0];
                        int dimention = strlen(res_msg);
                        while(dimention > 0){
                            n_write = write(newfd, ptr, dimention);
                            if(n_write <= 0){
                                perror("Write failed");
                                exit(EXIT_FAILURE);
                            }
                            dimention -= n_write; 
                            ptr += n_write;
                        }
                        free(buffer);
                        free(res_msg);
                        close(newfd);
                        closedir(DIR_games);
                        closedir(DIR_scores);
                        exit(EXIT_SUCCESS);
                    }
                }
                if(FD_ISSET(fd_udp,&testfds)){
                    
                    char *buffer = (char*) calloc(128, 1);
                    char PLID[7];

                    addrlen=sizeof(addr);
                    if(recvfrom(fd_udp,buffer,128,0,(struct sockaddr*)&addr,&addrlen) == -1)/*error*/
                        exit(EXIT_FAILURE);
                    
                    command = strtok(buffer, " ");
                    arguments = strtok(NULL, "");

                    sscanf(arguments,"%6s", PLID);

                    if (verbose_mode == 1)
                        printf("UDP -> PLID:%s, Request:%s, IP:%s, PORT:%d\n", PLID, 
                            command, (char*)inet_ntoa((struct in_addr)addr.sin_addr), ntohs(addr.sin_port));

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
                    free(buffer);
                }
        }
    }
    
    close(fd_udp);
    close(fd_tcp);

    exit(EXIT_SUCCESS);
}