#include "Server.h"
#include "Client.h" 
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

int verbose_mode = 0; // 0 desativado; 1 ativado

int main(int argc, char *argv[]){
    char *port;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    int fd,errcode;
    ssize_t n;
    char buffer[128];

    if (argc <= 0 || argc > 4){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1){
        // Port 58000 + nº Grupo(14)
        port = "58014";
    }
    if (argc == 2){
        if (strcmp(argv[1],"-p"))
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

    //PARA UDP
    fd=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
    if(fd==-1)/*error*/
        exit(1);

    memset(&hints,0,sizeof hints);
    hints.ai_family= AF_UNSPEC;//IPv4
    hints.ai_socktype=SOCK_DGRAM;//UDP socket
    hints.ai_flags = AI_PASSIVE;

    errcode=getaddrinfo(NULL,port,&hints,&res);
    if(errcode!=0){/*error*/
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errcode));
        return 1;
    }
    n = bind(fd,res->ai_addr, res->ai_addrlen);
    if(n==-1)/*error*/
        exit(1);
    while (1){
        addrlen=sizeof(addr);
        n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
        if(n==-1)/*error*/
            exit(1);
        printf("%s\n",buffer);

        // write(1,"received: ",10);//stdout
        // write(1,buffer,n);
        // n=sendto(fd,buffer,n,0,(struct sockaddr*)&addr,addrlen);
        // if(n==-1)/*error*/
        //     exit(1);
    }
    freeaddrinfo(res);

    exit(EXIT_SUCCESS);
}