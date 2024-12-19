#include "Client.h"
#include "extra_player.h"
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>


char *port, *ip_address;
char plId[6];
int nT = 1;



void reset_para(){
    nT = 1;
    return;
}



void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n");
        EXIT();
    }
}



void start(char* arguments){ //UDP protocol

    char* res_msg = (char*) calloc(8,1);
    char *protocol;
    char msg[17];
    
    memset(plId,0,sizeof(plId));

    snprintf(msg, sizeof(msg), "SNG %s", arguments);
    
    if (UDP(msg,ip_address,port,res_msg) == 1){
        free(res_msg);
        return;
    }

    strtok(res_msg," ");
    protocol = strtok(NULL, "");

    if (strcmp(protocol,"ERR\n") == 0){
        fprintf(stdout, "Incorrect Arguments in fuction 'start'\n");
        free(res_msg);
        return;
    }
    else if (strcmp(protocol,"NOK\n") == 0)
        fprintf(stdout, "Game already Created\n");
    else
        fprintf(stdout, "Game successfully Created. GOOD LUCK!\n");

    for(int i = 0; i < 6; i++){
            plId[i] = arguments[i];
    }
    free(res_msg);
}



void TRY(char* arguments){ //UDP protocol
    /* red (R), green (G), blue (B), yellow (Y), orange (O) and purple (P)*/

    char* res_msg = (char*) calloc(22,1);
    char msg[22];
    char *protocol;
    
    if (strcmp(plId,"") == 0){
        fprintf(stdout, "There is no ongoing game.\n");
        return ;
    }

    snprintf(msg, sizeof(msg), "TRY %s %s %d\n", plId, strtok(arguments,"\n"), nT);
    
    if (UDP(msg,ip_address,port,res_msg) == 1){
        free(res_msg);
        return;
    }

    strtok(res_msg," ");
    protocol = strtok(NULL, " ");

    if (strcmp(protocol,"OK") == 0){
        char *endptr;
        int new_nt, nB, nW;

        new_nt = strtol(strtok(NULL, " "), &endptr, 10);
        nB = strtol(strtok(NULL, " "), &endptr, 10);
        nW = strtol(strtok(NULL, ""), &endptr, 10);

        if (new_nt == -1)
            nT--;

        fprintf(stdout, "Guess result: nB: %d, nW: %d, Num of Tries left: %d\n",nB,nW, 8 - nT);
        if(nB == 4){
            fprintf(stdout, "YOU WON. Guesses needed: %d. GOOD JOB!!!!\n",nT);
            reset_para();
        }
    }
    else if (strcmp(protocol,"DUP\n") == 0){
        fprintf(stdout, "Repeated guess. Try again!\n");
        nT--; 
    }
    else if (strcmp(protocol,"INV") == 0){
        fprintf(stdout, "Repeat the last 'TRY'!\n");
        nT--;
    }
    else if (strcmp(protocol,"NOK\n") == 0){
        fprintf(stdout, "Player does not have an ongoing game.\n");
        nT--;
    }
    else if (strcmp(protocol,"ENT") == 0){
        fprintf(stdout, "YOU LOST!! No more attempts available.\n");
        printf("Solu,tion: %sBetter luck next time.\n", strtok(NULL, ""));
        reset_para();
    }
    else if (strcmp(protocol,"ETM") == 0){
        fprintf(stdout, "YOU LOST!! Time ended.\n");
        printf("Solution: %sBetter luck next time.\n", strtok(NULL, ""));
        reset_para();
    }
    else if (strcmp(protocol,"ERR\n") == 0){
        fprintf(stdout, "Incorrect Arguments in fuction 'try'\n");
        nT--;
    }
    
    nT++;
    free(res_msg);
}



void show_trials(){ //TCP session

    char* res_msg = (char*) calloc(2049,1);
    char msg[12];
    char *protocol, *endptr;

    if (strcmp(plId,"") == 0){
        fprintf(stdout, "There is no ongoing/finished game.\n");
        return ;
    }
    
    snprintf(msg, sizeof(msg), "STR %s\n", plId);

    if(TCP(msg, ip_address, port, res_msg) == 1){
        printf("Maybe try again later?\n");
        return;
    }
    strtok(res_msg," ");
    protocol = strtok(NULL, " ");

    if (strcmp(protocol,"NOK\n") == 0){
        fprintf(stdout, "No game started\n");
    }
    else{
        char *fname, *fsize, *fdata;

        fname = strtok(NULL, " ");
        fsize = strtok(NULL, " ");
        fdata = strtok(NULL, "");

        FILE* fd = fopen(fname,"w+");

        ssize_t ret = fwrite(fdata, sizeof(char), strtol(fsize, &endptr, 10), fd);
        if (ret < 0) {
            fprintf(stderr, "Write failed\n");
            exit(EXIT_FAILURE);
        }

        ret = fwrite(fdata, sizeof(char), strtol(fsize, &endptr, 10), stdout);
        if (ret < 0) {
            fprintf(stderr, "Write failed\n");
            exit(EXIT_FAILURE);
        }
        if (strcmp(protocol,"FIN") == 0)
            reset_para();
        
        fclose(fd);  
    }
    free(res_msg);
}



void scoreboard(){ //TCP session

    char* res_msg = (char*) calloc(2049,1);
    char msg[5];
    char *protocol, *endptr;
    
    snprintf(msg, sizeof(msg), "SSB\n");

    if(TCP(msg, ip_address, port, res_msg) == 1){
        printf("Maybe try again later?\n");
        return;
    }
    strtok(res_msg," ");
    protocol = strtok(NULL, " ");
    if (strcmp(protocol,"EMPTY\n") == 0)
        fprintf(stdout, "No game have been finished\n");
    
    else{
        char *fname, *fsize, *fdata;

        fname = strtok(NULL, " ");
        fsize = strtok(NULL, " ");
        fdata = strtok(NULL, "");
        
        FILE* fd = fopen(fname,"w");

        ssize_t ret = fwrite(fdata, sizeof(char), strtol(fsize, &endptr, 10), fd);
        if (ret < 0) {
            fprintf(stderr, "Write failed\n");
            exit(EXIT_FAILURE);
        }

        printf("%s", fdata);
        fclose(fd);  
    }
    free(res_msg);
}



int quit(){ //UDP protocol

    char* res_msg = (char*) calloc(15,1);
    char *protocol, *result;
    char msg[12];
    
    if (strcmp(plId,"") == 0){
        fprintf(stdout, "There is no ongoing game.\n");
        return 0;
    }

    snprintf(msg, sizeof(msg), "QUT %s\n",plId);
    
    if (UDP(msg,ip_address,port,res_msg) == 1){
        free(res_msg);
        return 1;
    }
    
    strtok(res_msg," ");
    protocol = strtok(NULL, " ");

    if(strcmp(protocol, "ERR\n") == 0){
        fprintf(stdout, "Error while quitting the game.\n");
        free(res_msg);
        return 1;
    }
    else if (strcmp(protocol,"OK") == 0){
        result = strtok(NULL, "");
        printf("Solution: %s", result);
    }
    else if (strcmp(protocol, "NOK\n") == 0)
        fprintf(stdout, "There is no ongoing game.\n");

    free(res_msg);
    return 0;    
}



void EXIT(){ //UDP protocol

    if (strcmp(plId,"") == 0)
        exit(EXIT_SUCCESS);
    
    if (quit() == 0){
        fprintf(stdout, "Leaving application. See you next time player!!!\n");
        exit(EXIT_SUCCESS);
    }

}



void debug(char *arguments){ //UDP protocol
    
    char* res_msg = (char*) calloc(8,1);
    char *protocol;
    char msg[24];

    memset(plId,0,sizeof(plId));

    snprintf(msg, sizeof(msg), "DBG %s", arguments);
    
    if (UDP(msg,ip_address,port,res_msg) == 1){
        free(res_msg);
        return;
    }

    strtok(res_msg," ");
    protocol = strtok(NULL, "");

    if (strcmp(protocol,"ERR\n") == 0){
        fprintf(stdout, "Incorrect Arguments in fuction 'Debug'\n");
        free(res_msg);
        return;
    }
    else if (strcmp(protocol,"NOK\n") == 0)
        fprintf(stdout, "Game already Created\n");
    else
        fprintf(stdout, "Game successfully Created. GOOD LUCK!\n");
    

    for(int i = 0; i < 6; i++){
            plId[i] = arguments[i];
    }
    free(res_msg);
}



int main(int argc, char *argv[]){
    char *arguments;
    char input[128];
    struct sigaction act;

    if (argc != 1 && argc != 3 && argc != 5){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1){
        ip_address = getIPaddress();// IP DO NOSSO PC
        port = "58014";// Port 58000 + nº Grupo(14)
    }
    else if (argc == 3){
        if (strcmp(argv[1],"-n") == 0){
            ip_address = argv[2];
            port = "58014";// Port 58000 + nº Grupo(14)
        }       
        else{
            ip_address = getIPaddress();// IP DO NOSSO PC
            port = argv[2];
        }
    }
    else{
        ip_address = argv[2];
        port = argv[4];
    }

    memset(&act, 0, sizeof act);
    act.sa_handler = &handle_signal;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("Sigaction failed");
        exit(EXIT_FAILURE);
    }

    while (1){ 
        fgets(input, sizeof(input), stdin);

        strtok(input," ");
        arguments = strtok(NULL, "");

        if (strcmp(input,"start") == 0){
            start(arguments);
        }
        else if (strcmp(input,"try") == 0){
            TRY(arguments);
        }
        else if (strcmp(input,"show_trials\n") == 0 || strcmp(input,"st\n") == 0){
            show_trials();
        }
        else if (strcmp(input,"scoreboard\n") == 0 || strcmp(input,"sb\n") == 0){
            scoreboard();
        }
        else if (strcmp(input,"quit\n") == 0){
            quit();
        }
        else if (strcmp(input,"exit\n") == 0){
            EXIT();
        }
        else if (strcmp(input,"debug") == 0){
            debug(arguments);
        }
        else
            fprintf(stdout, "Incorrect Command\n");
        
        memset(input, 0, strlen(input));
    }

    exit(EXIT_SUCCESS);
}