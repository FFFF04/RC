#include "Client.h"
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
        fprintf(stdout, "Incorrect Arguments in fuction 'start'\n");
    else if (strcmp(protocol,"NOK\n") == 0){
        fprintf(stdout, "Game already Created\n");
        for(int i = 0; i < 6; i++){
            plId[i] = arguments[i];
        }
    }
    else{
        fprintf(stdout, "Game successfully Created. GOOD LUCK!\n");
        for(int i = 0; i < 6; i++){
            plId[i] = arguments[i];
        }
    }
    free(res_msg);
}



void TRY(char* arguments){ //UDP protocol
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

    char* res_msg = (char*) calloc(22,1);
    char msg[22];
    char *protocol;
    
    snprintf(msg, sizeof(msg), "TRY %s %s %d\n", plId, strtok(arguments,"\n"), nT);
    UDP(msg,ip_address,port,res_msg);
    
    strtok(res_msg," ");
    protocol = strtok(NULL, " ");

    if (strcmp(protocol,"OK") == 0){
        char *endptr;
        int new_nt, nB, nW;

        new_nt = strtol(strtok(NULL, " "), &endptr, 10);
        nB = strtol(strtok(NULL, " "), &endptr, 10);
        nW = strtol(strtok(NULL, ""), &endptr, 10);

        if (new_nt == -1){ 
            // Foi um resent pois o server nao conseguio enviar a 
            // mensagem e tentamos outra vez enviar pelo terminal a mesma mensagem
            nT--;
        }
        //Imprimimos sempre mesmo que ganhemos acho que fica bem :)
        fprintf(stdout, "Guess result: nB(colour and position correct): %d, nW(colour correct): %d, Num of Tries left: %d\n",nB,nW, 7 - nT);
        if(nB == 4){
            fprintf(stdout, "YOU WON. Guesses needed: %d. GOOD JOB!!!!\n",nT);
            memset(plId,0,sizeof(plId));
            nT = 0;
        }
    }
    else if (strcmp(protocol,"DUP\n") == 0){
        fprintf(stdout, "Repeated guess. Try again!\n");
        nT--; 
    }
    else if (strcmp(protocol,"INV") == 0){ // Acho que é so escrever mensagem.
        // Escrever tipo problemas a enviar para o servidor por favor reenviar o try anterio assim uma cena
        nT--;
    }
    else if (strcmp(protocol,"NOK\n") == 0){
        fprintf(stdout, "Player does not have an ongoing game.\n");
        nT--;
    }
    else if (strcmp(protocol,"ENT") == 0){
        fprintf(stdout, "YOU LOST!! No more attempts available.\n");
        printf("Solution: %sBetter luck next time.\n", strtok(NULL, ""));
        memset(plId,0,sizeof(plId));
        nT = 0;
    }
    else if (strcmp(protocol,"ETM") == 0){
        fprintf(stdout, "YOU LOST!! Time ended.\n");
        printf("Solution: %sBetter luck next time.\n", strtok(NULL, ""));
        memset(plId,0,sizeof(plId));
        nT = 0;
    }
    else if (strcmp(protocol,"ERR\n") == 0){
        fprintf(stdout, "Incorrect Arguments in fuction 'try'\n"); //acho que dá para escrever este erro de maneira mais "normal"
        nT--;
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
        fprintf(stdout, "No Game Started yet!!!!\n");
        return;
    }
    printf("%s",arguments);

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
        fprintf(stdout, "No Game Started yet!!!!\n");
        return;
    }
    printf("%s",arguments);
}



void quit(int exit_status){ //UDP protocol
    /*the player can ask to terminate the game at any moment. If a game was
    under way, the GS server should be informed, by sending a message using the
    UDP protocol.*/

    char* res_msg = (char*) calloc(15,1);
    char *protocol, *result;
    char msg[12];

    snprintf(msg, sizeof(msg), "QUT %s\n", plId);
    UDP(msg,ip_address,port,res_msg);

    strtok(res_msg," ");
    protocol = strtok(NULL, " ");
    
    if (strcmp(protocol,"OK") == 0){
        result = strtok(NULL, "");
        printf("Solution: %s",result);
    }
    else if (strcmp(protocol, "NOK\n") == 0){
        fprintf(stdout, "No have an ongoing game.\n");
        exit_status = 0;
    }
    else if(strcmp(protocol, "ERR\n") == 0){
        fprintf(stdout, "Error while quitting the game.\n");
        exit_status = 0;
    }
    
    free(res_msg);
    memset(plId,0,sizeof(plId));
    nT = 1;
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
    char msg[24];  
    snprintf(msg, sizeof(msg), "DBG %s", arguments);
    UDP(msg,ip_address,port,res_msg);

    strtok(res_msg," ");
    protocol = strtok(NULL, "");

    if (strcmp(protocol,"ERR\n") == 0)
        fprintf(stdout, "Incorrect Arguments in fuction 'start'\n");
    else if (strcmp(protocol,"NOK\n") == 0)
        fprintf(stdout, "Game already Created\n");
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
            TRY(arguments);
        }
        else if (strcmp(input,"show_trials\n") == 0 || strcmp(input,"st\n") == 0){
            show_trials(arguments);
        }
        else if (strcmp(input,"scoreboard\n") == 0 || strcmp(input,"sb\n") == 0){
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
        else
            fprintf(stdout, "Incorrect Command\n");
        
        memset(input, 0, strlen(input));
    }

    exit(EXIT_SUCCESS);
}