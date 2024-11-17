#include "Client.h"
#include "Server.h"
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



void start(char* PLID, int max_playtime){ //UDP protocol
    /* following this command the Player application sends a 
    message to the GS, using the UDP protocol, asking to start a
    new game, provides the player identification PLID and indicates the
    max_playtime value, in seconds, in which the player proposes to complete
    the game (it cannot exceed 600 seconds).
    The GS randomly selects a 4 colour key: C1 C2 C3 C4 and informs the player
    that it can start playing. The Player application displays this information. */

    //temos de criar file e dar erro se ja existir jogo nao acabado
}



void try(char C1, char C2, char C3, char C4){ //UDP protocol
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

    // temos de dar erro se nao existir nenhum jogo ativo
}



void show_trials(){ //TCP session
    /*following this command the Player establishes a
    TCP session with the GS and sends a message asking to receive a list of
    previously made trials and the respective results. In reply, the GS sends a text
    file containing the requested list (including a line for each trial: C1 C2 C3 C4
    nB nW) and the remaining playing time. After receiving the reply from the GS,
    the list of trials and the corresponding results is displayed by the Player
    application.
    */
}



void scoreboard(){ //TCP session
    /*following this command the Player establishes a TCP
    session with the GS and sends a message asking to receive an updated
    scoreboard. In reply, the GS sends a text file containing the top 10 scores
    (including for each: PLID, number of plays to win, secret key) – the file only
    contains scores for games where the user won the game and discovered the
    secret key. After receiving the reply from the GS, the scoreboard is displayed as
    a numbered list.*/
}



void quit(){ //UDP protocol
    /*the player can ask to terminate the game at any moment. If a game was
    under way, the GS server should be informed, by sending a message using the
    UDP protocol.*/


    //temos de apagar file acho eu
}



void EXIT(){ //UDP protocol
    /*the player asks to exit the Player application. If a game was under way,
    the GS server should be informed, by sending a message using the UDP protocol.
    */
}



void debug(char* PLID, int max_playtime, char C1, char C2, char C3, char C4){ //UDP protocol
    /*following this command
    the Player starts a game in debug mode. It sends a message to the GS, using the
    UDP protocol, asking to start a new game, providing the player identification
    PLID, indicating the max_playtime value, and specifying the secret key to
    be used: C1 C2 C3 C4.
    When the GS receives this request, it checks if this player already has any
    ongoing game, and if a new game can be started the GS uses the secret key
    provided in the message. The Player application is informed that it can start
    playing.*/
}



int main(int argc, char *argv[]){
    char *port, *ip_address, *input;

    if (argc != 1 && argc != 3 && argc != 5){
        fprintf(stderr, "Incorrect Arguments\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 1){
        // IP DO NOSSO PC
        ip_address = getIPadress();

        // Port 58000 + nº Grupo(14)
        port = "58014";

        //printf("%s\n%s\n",ip_address, port);
    }
    if (argc == 3){
        if (strcmp(argv[1],"-n")){
            ip_address = argv[2];
            // Port 58000 + nº Grupo(14)
            port = "58014";
        }       
        else{
            // IP DO NOSSO PC
            ip_address = getIPadress();
            port = argv[2];
        }
        //printf("%s\n%s\n",ip_address, port);
    }
    else{
        ip_address = argv[2];
        port = argv[4];
        //printf("%s\n%s\n",ip_address, port);
    }

    while (1){
        fgets(input, sizeof(input), stdin);

        char *command = strtok(input, " ");

        if (strcmp(command,"start")){
            
            continue;
        }
        if (strcmp(command,"try")){
            
            continue;
        }
        if (strcmp(command,"quit")){
            
            continue;
        }
        if (strcmp(command,"exit")){
            
            continue;
        }
        
        if (strcmp(command,"debug")){
            
            continue;
        }

        
    }

    exit(EXIT_SUCCESS);
}