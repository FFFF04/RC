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
#include <sys/stat.h>

#define MAX_TRIES 4 

DIR *SearchOrCreateGameDir(const char *parent_dir, int PLID) {
    char target_path[256];
    struct stat sb_games;
    DIR *dp;

    // Construct the target directory path
    snprintf(target_path, sizeof(target_path), "%s/GAME_%d", parent_dir, PLID);

    // Check if the directory exists
    if (stat(target_path, &sb_games) == -1) {
        // Directory does not exist, create it
        if (mkdir(target_path, 0777) == 0) {
            printf("Directory '%s' created successfully.\n", target_path);
        } 
        else {
            perror("mkdir");
            return NULL;
        }
    } 
    else 
        printf("Path '%s' already exists, continuing.\n", target_path);

    // Open the directory
    dp = opendir(target_path);
    if (dp == NULL) {
        perror("opendir");
        return NULL;
    }

    return dp;
}




FILE *SearchOrCreateGameFile(const char *directory, int PLID, int *created) {
    char filepath[256];
    struct stat sb;
    FILE *file;

    // Construct the file path
    snprintf(filepath, sizeof(filepath), "%s/GAME_%d.txt", directory, PLID);

    // Check if the file exists
    if (stat(filepath, &sb) == 0) {
        // File exists, open it for reading and writing
        printf("File '%s' found, opening.\n", filepath);
        file = fopen(filepath, "r+");
        if (file == NULL) {
            perror("fopen");
            return NULL;
        }
        created = 1;
    } 
    else {
        // File does not exist, create it
        printf("File '%s' not found, creating new game file.\n", filepath);
        file = fopen(filepath, "w+");
        if (file == NULL) {
            perror("fopen");
            return NULL;
        }

        // Initialize the new game file with default data if necessary
        fprintf(file, "New game created for PLID: %d\n", PLID);
        fflush(file);
        created = 0;
    }

    return file;
}



void WriteGameStart(FILE *game_file, int PLID, char mode, const char *color_code, int time_limit) {
    time_t raw_time;
    struct tm *time_info;
    char date[11];  // Format: YYYY-MM-DD
    char time_str[9]; // Format: HH:MM:SS

    // Get the current time
    time(&raw_time);
    time_info = localtime(&raw_time);

    // Format the date and time
    strftime(date, sizeof(date), "%Y-%m-%d", time_info);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);

    // Write the formatted data to the game file
    fprintf(game_file, "%d %c %s %d %s %s %ld\n", 
            PLID, mode, color_code, time_limit, date, time_str, raw_time);

    // Flush the file to ensure data is written
    fflush(game_file);
}



int FindLastGame(char *PLID, char *fname) {
    struct dirent **filelist;
    int nentries, found;
    char dirname[20];

    // Construct the directory path
    sprintf(dirname, "GAMES/%s/", PLID);

    // Scan the directory and sort files alphabetically
    nentries = scandir(dirname, &filelist, 0, alphasort);
    found = 0;

    // If no entries are found, return 0
    if (nentries <= 0) {
        return 0;
    } else {
        // Iterate through the entries from last to first
        while (nentries--) {
            // Check if the entry is not a hidden file and if no game is found yet
            if (filelist[nentries]->d_name[0] != '.' && !found) {
                // Construct the file path of the last game
                sprintf(fname, "GAMES/%s/%s", PLID, filelist[nentries]->d_name);
                found = 1;
            }
            // Free the memory allocated for the entry
            free(filelist[nentries]);
        }
        // Free the file list
        free(filelist);
    }

    return found;
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
    if (nentries <= 0) {
        return 0;
    } else {
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
                           list->colcode[ifile],
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
    list->nscores = ifile;

    return ifile;
}












char* getIPaddress()
{
    char *ip_address;
    struct ifaddrs *ifaddr, *tmp;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    tmp = ifaddr;

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
            printf("No response received from the server, please recent the same command\n");
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




void TCP(char* line, char* ip_address, char* port, char* msg) {
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

    freeaddrinfo(res);

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

    nleft = 1000; // Max buffer size for receiving
    while (1) {
        nread = read(fd, msg, nleft);
        if (nread == -1) {
            perror("read failed");
            close(fd);
            exit(EXIT_FAILURE);
        } 
        else if (nread == 0)
            break;

        nleft -= nread;
        msg += nread;
    }
    close(fd);
}