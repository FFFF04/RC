#ifndef EXTRA_PLAYER_H_
#define EXTRA_PLAYER_H_

#include <stddef.h>
#include <stdio.h>

#define MODEPLAY 1
#define MODEDEBUG 2
#define MAX_TRIES 4

char* getIPaddress();
void send_msg(int file, char const *str);
void read_msg(char *prod_consumidor, int file, size_t size);
int UDP(char* line, char* ip_address, char* port, char* msg);
int TCP(char* line, char* ip_address, char* port, char* msg);

#endif
