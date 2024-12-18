#ifndef EXTRA_SERVER_H_
#define EXTRA_SERVER_H_

#include <stddef.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>

#define MODEPLAY 1
#define MODEDEBUG 2

typedef struct {
    int score[10];
    char PLID[10][7];
    char color_code[10][5];
    int ntries[10];
    int mode[10];
} SCORELIST;

int arguments_number(char *arguments);
DIR *SearchAndCreateGameDir(const char *parent_dir, int PLID);
void removeFile(FILE* game_file, char* directory, int num_PLID);
int CheckGameFileExists(const char *directory, int PLID, int protect);

FILE *CreateAndOpenGameFile(const char *directory, int PLID, char* open_type);

int CreateTimestampedFile_TRY(const char *directory, char *first_line, char *rest_file, 
    char code, struct tm *time_info, int duration);
void CreateTimestampedFile_QUIT(const char *directory, char *first_line, char *rest_file, char* res_msg);

void WriteGameStart(FILE *game_file, int PLID, char *mode, const char *color_code, int time_limit);
void calculate_blacks_and_whites(char *key, char *key_sol, int *nB, int *nW);
int CreateFile_SCORE(int num_PLID, int score, struct tm *time_info, char *color_code, long int num_nt, char mode);
int FindLastGame(int PLID, char *fname);
int calculate_file_size(char *Fdata, char *last_line);
int FindTopScores(SCORELIST *list);
int CalculateScore(int rank, int duration, int max_duration);

#endif