//#include "Client.h"
#include "extra.h"
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


char *port, *ip_address;
char plId[6];
int nT = 1;


void start(char* arguments){ //UDP protocol
    /* following this command the Player application sends a 
    message to the GS, using the UDP protocol, asking to start a
    new game, provides the player identification PLID and indicates the
    max_playtime value, in seconds, in which the player proposes to complete
    the game (it cannot exceed 600 seconds).
    The GS randomly selects a 4 colour key: C1 C2 C3 C4 and informs the player
    that it can start playing. The Player application displays this information. */
    
    char* res_msg = (char*) calloc(8,1);
    char *protocol;
    char msg[17];  
    snprintf(msg, sizeof(msg), "SNG %s", arguments);
    UDP(msg,ip_address,port,res_msg);

    strtok(res_msg," ");
    protocol = strtok(NULL, "");

    if (strcmp(protocol,"ERR\n") == 0)
        fprintf(stderr, "Incorrect Arguments in fuction 'start'\n");
    else if (strcmp(protocol,"NOK\n") == 0)
        fprintf(stderr, "Game already Created\n");
    else{
        fprintf(stdout, "Game successfully Created. GOOD LUCK!\n");
        for(int i = 0; i < 6; i++){
            plId[i] = arguments[i];
        }
    }
    
    free(res_msg);
    //temos de criar file e dar erro se ja existir jogo nao acabado
}



void try(char* arguments){ //UDP protocol
    /* red (R), green (G), blue (B), yellow (Y), orange (O) and purple (P)*/

    /*the Player application sends a message to the GS,
    using the UDP protocol, asks to check if C1 C2 C3 C4 is the secret key to be
    guessed. If the maximum number of trials has been exceeded the player loses the
    game, otherwise the number of trials is increased. If the maximum playtime
    (max_playtime) has been reached the player loses the game.
    Otherwise, the GS replies informing the number of Ci guesses that are correct in
    both colour and position (nB), and the number of Ci guesses that belong to the
    secret key but are incorrectly positioned (nW). If nB = 4 the secret code has
    been correctly guessed and the player wins the game. The Player application
    displays the received information.*/

    if (strcmp(plId,"") == 0){ //Isto provavelmente n funciona pq não iniciamos a vazio
        fprintf(stderr, "No Game Started yet!!!!\n");
        return;
    }
    char* res_msg = (char*) calloc(22,1);
    char msg[22];
    char *protocol;
    char *result;
    
    
    snprintf(msg, sizeof(msg), "TRY %s %s %d\n", plId, strtok(arguments,"\n"), nT);

    printf("%s",msg);

    UDP(msg,ip_address,port,res_msg);

    strtok(res_msg," ");
    protocol = strtok(NULL, " ");
    result = strtok(NULL, "");
    
    if (strcmp(protocol,"OK") == 0);
        // ver o que fazer com o result
        /*
        Pode acontecer 2 casos:
            OK, YOU WON!!! se nB = 4 ou nT = -1
            Mas tipo como é que é igual ao anterior se for igual ao anterior e o anterior foi OK
        ou
            OK, NOT WON YET. TRY AGAIN! 
        */

    else if (strcmp(protocol,"DUP") == 0){
        fprintf(stderr, "Repeated guess. Try again!\n");
        nT--; 
    }
    else if (strcmp(protocol,"INV") == 0){
        //nao tenho a certeza deste
    }
    else if (strcmp(protocol,"NOK") == 0){
        fprintf(stderr, "Player %s does not have an ongoing game.\n", plId); //acho que pode haver mais casos
    }
    else if (strcmp(protocol,"ENT") == 0){
        fprintf(stderr, "No more attempts available.\n"); //revelar chave vencedora
    }
    else if (strcmp(protocol,"ETM") == 0){
        fprintf(stderr, "Time ended.\n"); //revelar chave vencedora
    }
    else if (strcmp(protocol,"ERR") == 0){
        fprintf(stderr, "Incorrect Arguments in fuction 'try'\n"); //acho que dá para escrever este erro de maneira mais "normal"
    }
    
    


    nT++;
    free(res_msg);
}



void show_trials(char *arguments){ //TCP session
    /*following this command the Player establishes a
    TCP session with the GS and sends a message asking to receive a list of
    previously made trials and the respective results. In reply, the GS sends a text
    file containing the requested list (including a line for each trial: C1 C2 C3 C4
    nB nW) and the remaining playing time. After receiving the reply from the GS,
    the list of trials and the corresponding results is displayed by the Player
    application.
    */

    if (strcmp(plId,"") == 0){
        fprintf(stderr, "No Game Started yet!!!!\n");
        return;
    }

}



void scoreboard(char *arguments){ //TCP session
    /*following this command the Player establishes a TCP
    session with the GS and sends a message asking to receive an updated
    scoreboard. In reply, the GS sends a text file containing the top 10 scores
    (including for each: PLID, number of plays to win, secret key) – the file only
    contains scores for games where the user won the game and discovered the
    secret key. After receiving the reply from the GS, the scoreboard is displayed as
    a numbered list.*/

    if (strcmp(plId,"") == 0){
        fprintf(stderr, "No Game Started yet!!!!\n");
        return;
    }
}



void quit(int exit_status){ //UDP protocol
    /*the player can ask to terminate the game at any moment. If a game was
    under way, the GS server should be informed, by sending a message using the
    UDP protocol.*/

    char* res_msg = (char*) calloc(22,1);
    char *protocol, *result;
    char msg[22];

    snprintf(msg, sizeof(msg), "QUT %s\n", plId);
    //UDP(msg,ip_address,port,res_msg);
    //FALTA Analise do RES_MSG

    strtok(res_msg," ");
    protocol = strtok(NULL, " ");
    result = strtok(NULL, "");

    if (strcmp(protocol,"OK") == 0)
        //revelar secret key
        printf("Solution: %s\n",result);

    else if (strcmp(protocol, "NOK") == 0){
        fprintf(stderr, "No have an ongoing game.\n"); //acho que pode haver mais casos
        exit_status = 0;
    }

    else if(strcmp(protocol, "ERR") == 0){
        fprintf(stderr, "Error while quitting the game.\n");
        exit_status = 0;
    }
    
    
    free(res_msg);
    memset(plId,0,sizeof(plId));
    if(exit_status == 1)
        exit(EXIT_SUCCESS);

}



void EXIT(){ //UDP protocol
    /*the player asks to exit the Player application. If a game was under way,
    the GS server should be informed, by sending a message using the UDP protocol.
    */

    if (strcmp(plId,"") == 0){
        exit(EXIT_SUCCESS);
    }
    
    quit(1);
}



void debug(char *arguments){ //UDP protocol
    /*following this command
    the Player starts a game in debug mode. It sends a message to the GS, using the
    UDP protocol, asking to start a new game, providing the player identification
    PLID, indicating the max_playtime value, and specifying the secret key to
    be used: C1 C2 C3 C4.
    When the GS receives this request, it checks if this player already has any
    ongoing game, and if a new game can be started the GS uses the secret key
    provided in the message. The Player application is informed that it can start
    playing.*/
    
    /* following this command the Player application sends a 
    message to the GS, using the UDP protocol, asking to start a
    new game, provides the player identification PLID and indicates the
    max_playtime value, in seconds, in which the player proposes to complete
    the game (it cannot exceed 600 seconds).
    The GS randomly selects a 4 colour key: C1 C2 C3 C4 and informs the player
    that it can start playing. The Player application displays this information. */
    
    char* res_msg = (char*) calloc(8,1);
    char *protocol;
    char msg[23];  
    snprintf(msg, sizeof(msg), "DBG %s", arguments);
    printf("%s",msg);
    //UDP(msg,ip_address,port,res_msg);

    strtok(res_msg," ");
    protocol = strtok(NULL, "");

    if (strcmp(protocol,"ERR\n") == 0)
        fprintf(stderr, "Incorrect Arguments in fuction 'start'\n");
    else if (strcmp(protocol,"NOK\n") == 0)
        fprintf(stderr, "Game already Created\n");
    else{
        fprintf(stdout, "Game successfully Created. GOOD LUCK!\n");
        for(int i = 0; i < 6; i++){
            plId[i] = arguments[i];
        }
    }
    
    free(res_msg);


}



int main(int argc, char *argv[]){
    char *arguments;
    char input[128];
    int i = 0;

    if (argc != 1 && argc != 3 && argc != 5){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1){
        // IP DO NOSSO PC
        ip_address = getIPaddress();

        // Port 58000 + nº Grupo(14)
        port = "58014";
    }
    else if (argc == 3){
        if (strcmp(argv[1],"-n") == 0){
            ip_address = argv[2];
            // Port 58000 + nº Grupo(14)
            port = "58014";
        }       
        else{
            // IP DO NOSSO PC
            ip_address = getIPaddress();
            port = argv[2];
        }
    }
    else{
        ip_address = argv[2];
        port = argv[4];
    }
    while (1){ 
        fgets(input, sizeof(input), stdin);

        strtok(input," ");
        
        arguments = strtok(NULL, "");

        if (strcmp(input,"start") == 0){
            start(arguments);
        }
        else if (strcmp(input,"try") == 0){
            try(arguments);
        }
        else if (strcmp(input,"show_trials") == 0 || strcmp(input,"st") == 0){
            show_trials(arguments);
        }
        else if (strcmp(input,"scoreboard") == 0 || strcmp(input,"sb") == 0){
            scoreboard(arguments);
        }
        else if (strcmp(input,"quit\n") == 0){
            quit(0);
        }
        else if (strcmp(input,"exit\n") == 0){
            EXIT();
        }
        else if (strcmp(input,"debug") == 0){
            debug(arguments);
        }
        // .......... Faltam if acho eu
        else
            fprintf(stderr, "Incorrect Command\n");
        
        memset(input, 0, strlen(input));
    }

    exit(EXIT_SUCCESS);
}