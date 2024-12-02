#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>

#define max(A,B) ((A)>=(B)?(A):(B))

int verbose_mode = 0; // 0 desativado; 1 ativado
int game_started = 0; // 0 nao ha nenhum; fd há jogo e é esse o file
FILE *fptr;
char colors[6] = {'R', 'G', 'B', 'Y', 'O', 'P'};
long int clock_my = 0; 

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!! TEMOS DE FAZER UM RELOGIO !!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

char* start(char* arguments){

    srand((unsigned int) time(NULL));

    char* PLID, *time, *endptr;
    long int num_PLID, num_time;
    //char solution[4];
    
    if(game_started == 1)
        return "RSG NOK\n";
    
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
    
    //CRIAR FILE com PLID que vai ser o nome mas com o que a frente?
    // fptr = fopen(strcat(PLID,".txt"), "w");
    // if (fptr == NULL) { 
    //     fprintf(stderr, "Could not open file\n");
    //     exit(EXIT_FAILURE);
    // } 
    // for (size_t i = 0; i < 4; i++)
    //     solution[i] = colors[rand() % 6];
    // solution[4] = '\0';
    // fputs(solution,fptr);
    // fputs("\n",fptr);
    // fflush(fptr);
    game_started = 1;
    //inicar o relogio aqui com o num_time
    // POR AGORA:
    clock_my = num_time;

    return "RSG OK\n";
}




int main(int argc, char *argv[]){
    char *port, *command, *arguments;
    struct sockaddr_in addr;
    socklen_t addrlen;
    struct addrinfo hints_udp, hints_tcp,*res_tcp, *res_udp;
    struct sigaction act;
    int fd_tcp, fd_udp, errcode, select_fds;
    
    fd_set inputs, testfds;
    enum {idle,busy} state;
    char buffer[128];

    if (argc <= 0 || argc > 4){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1){
        // Port 58000 + nº Grupo(14)
        port = "58014";
    }
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

    //ver se existe jogo nao acabado se existir abrir logo o jogo
    //Ter variavel para ser mais facil verificar depois
    // se esta a 1 entao encontrou se nao esta a 0

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
        exit(1);//error
    memset(&hints,0,sizeof hints_tcp);
    hints_tcp.ai_family=AF_INET;//IPv4
    hints_tcp.ai_socktype=SOCK_STREAM;//TCP socket
    hints_tcp.ai_flags = AI_PASSIVE;
    errcode=getaddrinfo(NULL, port, &hints_tcp, &res_tcp);
    if(errcode!=0)/*error*/
        exit(1);
    if(bind(fd_tcp, res_tcp->ai_addr, res_tcp->ai_addrlen)==-1)/*error*/{
        perror("bind");
        exit(1);
    }
        
    if(listen(fd_tcp,5)==-1)/*error*/
        exit(1);

    FD_ZERO(&inputs); // Clear input mask
    FD_SET(fd_udp,&inputs); // Set UDP channel on
    FD_SET(fd_tcp,&inputs); // Set TCP channel on

    while(1){
        testfds=inputs; // Reload mask

        select_fds=select(FD_SETSIZE,&testfds,(fd_set *)NULL,(fd_set *)NULL,(struct timeval *) NULL);

        switch(select_fds)
        {
            case -1:
                perror("select");
                exit(1);
            default:
                if(FD_ISSET(fd_tcp,&testfds)){
                    
                }
                if(FD_ISSET(fd_udp,&testfds)){

                    addrlen=sizeof(addr);
                    if(recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen) == -1)/*error*/
                        exit(EXIT_FAILURE);
                    
                    command = strtok(buffer, " ");
                    arguments = strtok(NULL, "");
            
                    if (strcmp(command,"SNG") == 0){
                        char* res_msg = start(arguments);
                        if(sendto(fd, res_msg, strlen(res_msg), 0, (struct sockaddr*)&addr, addrlen) == -1)/*error*/
                            exit(EXIT_FAILURE);
                    }
                    else if (strcmp(command,"try") == 0){
                        
                    }
                    else if (strcmp(command,"QUT") == 0){
                    }
                    else if (strcmp(command,"DBG") == 0){
                    }
                    memset(buffer, 0, sizeof(buffer));
                            }
                    }
    }
    while (1){
        addrlen=sizeof(addr);
        if(recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen) == -1)/*error*/
            exit(EXIT_FAILURE);
        
        command = strtok(buffer, " ");
        arguments = strtok(NULL, "");
 
        if (strcmp(command,"SNG") == 0){
            char* res_msg = start(arguments);
            if(sendto(fd, res_msg, strlen(res_msg), 0, (struct sockaddr*)&addr, addrlen) == -1)/*error*/
                exit(EXIT_FAILURE);
        }
        else if (strcmp(command,"try") == 0){
            
        }
        else if (strcmp(command,"QUT") == 0){
        }
        else if (strcmp(command,"DBG") == 0){
        }
        memset(buffer, 0, sizeof(buffer));
    }
    freeaddrinfo(res);

    exit(EXIT_SUCCESS);
}