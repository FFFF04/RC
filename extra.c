#include "extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <ifaddrs.h>
#include <signal.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>


#define MAX_TRIES 4 

DIR *SearchAndCreateGameDir(const char *parent_dir, int PLID) {
    char target_path[256];
    struct stat sb_games;
    DIR *dp;

    snprintf(target_path, sizeof(target_path), "%s%d", parent_dir, PLID);

    if (stat(target_path, &sb_games) == -1) {
        if (mkdir(target_path, 0777) == 0);
        else {
            perror("mkdir");
            return NULL;
        }
    } 

    dp = opendir(target_path);
    if (dp == NULL) {
        perror("opendir");
        return NULL;
    }

    return dp;
}


void removeFile(FILE* game_file, char *directory ,int num_PLID){
    char filename[50];
    snprintf(filename, sizeof(filename), "%s%d.txt", directory, num_PLID);
    fclose(game_file);
    remove(filename);
}


int CheckGameFileExists(const char *directory, int PLID, int protect) {
    char filepath[256], dirpath[256], *buffer, *first_line, *rest_file;
    struct stat sb;
    FILE* game_file;
    size_t file_size;
    int duration;
    long int difference, start_time;
    time_t raw_time;


    snprintf(filepath, sizeof(filepath), "%s/GAME_%d.txt", directory, PLID);

    if (stat(filepath, &sb) == 0){

        if(protect == 1){
            game_file = CreateAndOpenGameFile("GAMES/GAME_", PLID, "r+");
            fseek(game_file, 0, SEEK_END);
            file_size = ftell(game_file);
            rewind(game_file);

            buffer = (char *)malloc(file_size + 1);
            if (fread(buffer, 1, file_size, game_file) != file_size) {
                perror("fread");
                free(buffer);
                return -1;
            }
            buffer[file_size] = '\0';
            first_line = strtok(buffer,"\n");
            rest_file = strtok(NULL,"");
            time(&raw_time);

            sscanf(first_line, "%*6s %*1c %*4s %d %*10s %*8s %ld", &duration, &start_time);

            difference = raw_time - start_time;
            if(duration <= difference){ 
                snprintf(dirpath, sizeof(dirpath), "GAMES/%d", PLID);

                DIR* DIR_player_games = SearchAndCreateGameDir("GAMES/", PLID);

                CreateTimestampedFile_TRY(dirpath, first_line, rest_file, 'T', localtime(&raw_time), duration);
                closedir(DIR_player_games);
                removeFile(game_file,"GAMES/GAME_",PLID);
                free(buffer);
                return 0;
            }
            free(buffer);
        }
        return 1; // File exists
    }
    return 0; // File does not exist
}



FILE *CreateAndOpenGameFile(const char *directory, int PLID, char* open_type) {
    char filepath[256];
    FILE *file;

    snprintf(filepath, sizeof(filepath), "%s%d.txt", directory, PLID);

    file = fopen(filepath, open_type);
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }
    return file;
}



void WriteGameStart(FILE *game_file, int PLID, char *mode, const char *color_code, int time_limit) {
    time_t raw_time;
    struct tm *time_info;
    char date[11];
    char time_str[9];

    time(&raw_time);
    time_info = localtime(&raw_time);

    strftime(date, sizeof(date), "%Y-%m-%d", time_info);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);

    fprintf(game_file, "%d %s %s %d %s %s %ld\n", 
            PLID, mode, color_code, time_limit, date, time_str, raw_time);

    fflush(game_file);
}



int CreateTimestampedFile_TRY(const char *directory, char *first_line, char *rest_file, 
    char code, struct tm *time_info, int duration) {
    char filename[256];
    long int start_time;
    char color_code[4];

    sscanf(first_line, "%*6s %*1c %4s %d %*10s %*8s %ld", color_code, &duration, &start_time);
   
    // Format the filename as YYYYMMDD_HHMMSS_Q.txt
    snprintf(filename, sizeof(filename), "%s/%04d%02d%02d_%02d%02d%02d_%c.txt", 
             directory,
             time_info->tm_year + 1900,   // Year
             time_info->tm_mon + 1,       // Month
             time_info->tm_mday,          // Day
             time_info->tm_hour,          // Hour
             time_info->tm_min,           // Minute
             time_info->tm_sec,           // Second
             code);

    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    
    fprintf(file,"%s\n",first_line); // File com o jogo
    if (rest_file != NULL)
        fprintf(file,"%s",rest_file);

    fprintf(file, "%04d-%02d-%02d %02d:%02d:%02d %d\n", 
        time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday, 
        time_info->tm_hour, time_info->tm_min, time_info->tm_sec, duration); // Ultima linha
    
    fflush(file);
    fclose(file);
    return 0;
}



void CreateTimestampedFile_QUIT(const char *directory, char *first_line, char *rest_file, char* res_msg) {
    char filename[256], color_code[5];
    time_t raw_time;
    struct tm *time_info;
    int duration,difference;
    long int start_time;
    char code;
    // Get the current time
    time(&raw_time);
    time_info = localtime(&raw_time);
        
    sscanf(first_line, "%*6s %*1c %4s %d %*10s %*8s %ld", color_code, &duration, &start_time);
    difference = raw_time - start_time;
    if(duration <= difference)
        difference = duration;
    code = 'Q';
    // Format the filename as YYYYMMDD_HHMMSS_Q.txt
    snprintf(filename, sizeof(filename), "%s/%04d%02d%02d_%02d%02d%02d_%c.txt", 
             directory,
             time_info->tm_year + 1900,   // Year
             time_info->tm_mon + 1,       // Month
             time_info->tm_mday,          // Day
             time_info->tm_hour,          // Hour
             time_info->tm_min,           // Minute
             time_info->tm_sec,           // Second
             code);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        sprintf(res_msg, "ERR");
        return;
    }
    fprintf(file,"%s\n",first_line); // File com o jogo
    if (rest_file != NULL)
        fprintf(file,"%s",rest_file);
    
    fprintf(file, "%04d-%02d-%02d %02d:%02d:%02d %d\n", 
        time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday, 
        time_info->tm_hour, time_info->tm_min, time_info->tm_sec, difference); // Ultima linha
    
    fflush(file);
    fclose(file);
    sprintf(res_msg, "OK %c %c %c %c", color_code[0], color_code[1], color_code[2], color_code[3]);
    return;
}




void calculate_blacks_and_whites(char *key, char *key_sol, int *nB, int *nW) {
    int check_color_key[4] = {0};
    int check_color_key_sol[4] = {0};
    
    *nB = 0;
    *nW = 0;

    for (int i = 0; i < 4; i++) {
        if (key_sol[i] == key[i]) {
            (*nB)++;
            check_color_key[i] = 0;
            check_color_key_sol[i] = 0;
        } 
        else{
            check_color_key[i] = 1;
            check_color_key_sol[i] = 1;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (check_color_key[i]) {
            for (int l = 0; l < 4; l++) {
                if (check_color_key_sol[l] && (key[i] == key_sol[l])) {
                    (*nW)++;
                    check_color_key_sol[l] = 0;
                    break;
                }
            }
        }
    }
}



int CreateFile_SCORE(int num_PLID, int score, struct tm *time_info, char *color_code, long int num_nt, char mode) {
    char filepath[512];
    FILE *file;
    const char *dir = "SCORES";
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0777) != 0) {
            perror("mkdir");
            return 1;
        }
    }
    snprintf(filepath, sizeof(filepath), "%s/%d_%d_%02d%02d%04d_%02d%02d%02d.txt",
             dir,
             score,
             num_PLID,
             time_info->tm_mday,
             time_info->tm_mon + 1,
             time_info->tm_year + 1900,
             time_info->tm_hour,
             time_info->tm_min,
             time_info->tm_sec);

    file = fopen(filepath, "w");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    // Write the content to the file
    if (mode == 'P')
        fprintf(file, "%d %d %s %ld PLAY\n", score, num_PLID, color_code, num_nt);
    
    else 
        fprintf(file, "%d %d %s %ld DEBUG\n", score, num_PLID, color_code, num_nt);

    fflush(file);
    fclose(file);

    return 0; // Success
}



int FindLastGame(int PLID, char *fname) {
    struct dirent **filelist;
    int nentries, found;
    char dirname[20];

    sprintf(dirname, "GAMES/%d/", PLID);

    nentries = scandir(dirname, &filelist, 0, alphasort);
    found = 0;

    if (nentries <= 0) {
        return 0;
    } else {
        while (nentries--) {
            if (filelist[nentries]->d_name[0] != '.' && !found) {
                sprintf(fname, "GAMES/%d/%s", PLID, filelist[nentries]->d_name);
                found = 1;
            }
            free(filelist[nentries]);
        }
        free(filelist);
    }

    return found;
}



int calculate_file_size(char *Fdata, char *last_line){

    int file_size;
    // FILE *file;
    
    // file = fopen("File_size_aux", "w+");
    // if (file == NULL) {
    //     perror("fopen");
    //     return -1;
    // }

    // fprintf(file,"%s%s",Fdata, last_line);

    // fseek(file, 0, SEEK_END);
    // file_size = ftell(file);

    // fclose(file);
    // remove("File_size_aux");

    char buffer[2049];
    memset(buffer,'\0',1);
    strcat(buffer,Fdata);
    strcat(buffer,last_line);

    file_size = strlen(buffer);
    
    return file_size;
}



int CalculateScore(int rank, int duration, int max_duration) {

    // Calculate rank factor (normalized between 0 and 1)
    double rank_factor = (9.0 - rank) / 8.0;

    // Calculate duration factor (normalized between 0 and 1)
    double duration_factor = 1.0 - ((double)duration / max_duration);

    // Combine factors and scale to 1-100
    int score = (int)(1 + (rank_factor * duration_factor) * 99);

    return score;
}



int FindTopScores(SCORELIST *list) {
    struct dirent **filelist;
    int nentries, ifile;
    char fname[300];
    FILE *fp;
    char mode[8];

    // Scan the directory and sort files alphabetically
    nentries = scandir("SCORES/", &filelist, 0, alphasort);
    // If no entries are found, return 0
    if (nentries <= 0)
        return 0;

    else{
        ifile = 0;

        // Iterate through the entries from last to first
        while (nentries--) {
            // Check if the entry is not a hidden file and we haven't reached the top 10
            if (filelist[nentries]->d_name[0] != '.' && ifile < 10) {
                // Construct the file path
                sprintf(fname, "SCORES/%s", filelist[nentries]->d_name);

                // Open the file for reading
                fp = fopen(fname, "r");
                if (fp != NULL) {
                    // Read data from the file
                    fscanf(fp, "%d %s %s %d %s",
                           &list->score[ifile],
                           list->PLID[ifile],
                           list->color_code[ifile],
                           &list->ntries[ifile],
                           mode);

                    // Parse the game mode
                    if (!strcmp(mode, "PLAY"))
                        list->mode[ifile] = MODEPLAY;
                    if (!strcmp(mode, "DEBUG"))
                        list->mode[ifile] = MODEDEBUG;

                    // Close the file
                    fclose(fp);
                    ++ifile;
                }
            }
            // Free the memory allocated for the entry
            free(filelist[nentries]);
        }
        // Free the file list
        free(filelist);
    }

    // Update the number of scores in the list
    return ifile;
}












char* getIPaddress()
{
    char *ip_address;
    // char hostbuffer[256];
    // struct hostent *host_entry;
    struct ifaddrs *ifaddr, *tmp;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    tmp = ifaddr;

    // gethostname(hostbuffer, sizeof(hostbuffer));
    // host_entry = gethostbyname(hostbuffer);
    // ip_address = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET){
            
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            char *address = inet_ntoa(pAddr->sin_addr);
            ip_address = address;
            break;
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(ifaddr);
    return ip_address;
}



int UDP(char* line, char* ip_address, char* port, char* msg) {
    struct addrinfo hints, *res;
    int fd, n;
    struct sigaction act;
    struct sockaddr_in addr;
    socklen_t addrlen;

    memset(&act, 0, sizeof act);
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == -1) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    // Create UDP socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket
    if (fd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout;
    timeout.tv_sec = 5;  // 5 seconds timeout
    timeout.tv_usec = 0; // 0 microseconds
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    n = getaddrinfo(ip_address, port, &hints, &res);
    if (n != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(n));
        close(fd);
        exit(EXIT_FAILURE);
    }
    int tries = 0;
    addrlen = sizeof(addr);
    while (1){

        if (tries == MAX_TRIES){
            printf("No response received from the server, please resent the same command\n");
            freeaddrinfo(res);
            close(fd);
            return 1;
        }
        
        n = sendto(fd, line, strlen(line), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) {
            perror("sendto failed");
            freeaddrinfo(res);
            close(fd);
            exit(EXIT_FAILURE);
        }

        n = recvfrom(fd, msg, 128, 0, (struct sockaddr*)&addr, &addrlen);
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK){
                tries++;
                if (tries != MAX_TRIES)
                    printf("Failed to send message to the server, trying again\n");
                continue;
            }
            else{ 
                perror("recvfrom failed");
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
        break;
    }

    freeaddrinfo(res);
    close(fd);
    return 0;
}


int TCP(char* line, char* ip_address, char* port, char* msg) {
    struct addrinfo hints, *res;
    int fd, n;
    ssize_t nbytes, nleft, nwritten, nread;
    struct sigaction act;

    memset(&act, 0, sizeof act);
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == -1) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    // Create a TCP socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    n = getaddrinfo(ip_address, port, &hints, &res);
    if (n != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(n));
        close(fd);
        exit(EXIT_FAILURE);
    }

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        perror("connect failed");
        freeaddrinfo(res);
        close(fd);
        exit(EXIT_FAILURE);
    }

    nbytes = strlen(line);
    nleft = nbytes;
    while (nleft > 0) {
        nwritten = write(fd, line, nleft);
        if (nwritten <= 0) {
            perror("write failed");
            close(fd);
            exit(EXIT_FAILURE);
        }
        nleft -= nwritten;
        line += nwritten;
    }

    nleft = 2049; // Max buffer size for receiving

    while (nleft > 0) {
        nread = read(fd, msg, nleft);
        if (nread == 0)
            break;
        if (nread < 0) {
            perror("read failed");
            freeaddrinfo(res);
            close(fd);
            return 1;
        }
        nleft -= nread;
        msg += nread;
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}