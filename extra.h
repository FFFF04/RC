#ifndef EXTRA_H_
#define EXTRA_H_

#include <stddef.h>

char* getIPaddress();
void send_msg(int file, char const *str);
void read_msg(char *prod_consumidor, int file, size_t size);
void UDP(char* line, char* ip_address, char* port, char* msg);
void TCP(char* line, char* ip_address, char* port, char* msg);

#endif

