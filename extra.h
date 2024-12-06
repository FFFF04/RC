#ifndef EXTRA_H_
#define EXTRA_H_

#include <stddef.h>
#include <dirent.h>

DIR *SearchOrCreateGameDir(const char *parent_dir, int PLID);
FILE *SearchOrCreateGameFile(const char *directory, int PLID, int *created);

int FindLastGame(char *PLID, char *fname);
int FindTopScores(SCORELIST *list);

char* getIPaddress();
void send_msg(int file, char const *str);
void read_msg(char *prod_consumidor, int file, size_t size);
int UDP(char* line, char* ip_address, char* port, char* msg);
void TCP(char* line, char* ip_address, char* port, char* msg);

#endif

