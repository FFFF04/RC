#ifndef CLIENT_H_
#define CLIENT_H_

void reset_para();
void handle_signal(int sig);
void start(char* arguments);
void TRY(char* arguments);
void show_trials();
void scoreboard();
int quit();
void EXIT();
void debug(char *arguments);

#endif