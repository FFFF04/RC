#include "extra_player.h"
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
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

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

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        exit(EXIT_FAILURE);
    }

    time_t start_time = time(NULL);

    while (nleft > 0) {
        nread = read(fd, msg, nleft);
        if (nread == 0)
            break;
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (time(NULL) - start_time > 20) {
                    printf("TCP not working. Timeout reached. Server is probably down\n");
                    freeaddrinfo(res);
                    close(fd);
                    return 1; 
                }
                continue;
            } 
            else {
                perror("read failed");
                freeaddrinfo(res);
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
        nleft -= nread;
        msg += nread;
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}