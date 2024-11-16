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

int verbose_mode = 0; // 0 desativado; 1 ativado

int main(int argc, char *argv[]){
    char *port;

    if (argc <= 0 || argc > 4){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1){
        // Port 58000 + nº Grupo(14)
        port = "58014";
        printf("%s\n", port);
    }
    if (argc == 2){
        if (strcmp(argv[1],"-p") == 0)
            port = argv[2];

        else{
            port = "58014";     // Port 58000 + nº Grupo(14)
            verbose_mode = 1;
        }
        printf("%s\n", port);
    }
    else{
        port = argv[2];
        verbose_mode = 1;
        printf("%s\n", port);
    }

    exit(EXIT_SUCCESS);
}