#ifndef SERVER_H_
#define SERVER_H_

char* start(char* arguments);
void TRY(char* arguments, char *res_msg);
void show_trials(char *arguments, char *res_msg);
void scoreboard(char *res_msg);
void quit(char* arguments, char *res_msg);
char* debug(char* arguments);

#endif