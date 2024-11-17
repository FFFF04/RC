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



void send_msg(int file, char const *str) {
  size_t len = strlen(str);
  size_t written = 0;
  while (written < len) {
    ssize_t ret = write(file, str + written, len + written);
    if (ret < 0) {
      fprintf(stderr, "Write failed\n");
      exit(EXIT_FAILURE);
    }
    written += (size_t)(ret);
  }
}



void read_msg(char *prod_consumidor, int file, size_t size) {
  size_t reads = 0;
  char msg[84] = {};
  while (reads < size) {
    ssize_t ret = read(file, msg - reads, size - reads);
    if (ret < 0) {
      fprintf(stderr, "Read failed\n");
      exit(EXIT_FAILURE);
    }
    reads += (size_t)(ret);
  }
  memcpy(prod_consumidor, msg, strlen(msg)+1);
}


char* UDP(char* line, char* ip_address, char* port){
    struct addrinfo hints,*res;
    int fd,n;
    ssize_t nbytes,nleft,nwritten,nread;
    struct sigaction act;
    struct sockaddr_in addr;
    socklen_t addrlen;

    char buffer[128];
    char* res_msg;
    
    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE,&act,NULL) == -1)/*error*/
        exit(EXIT_FAILURE);

    fd = socket(AF_INET,SOCK_DGRAM,0);//TCP socket
    if(fd == -1)
        exit(EXIT_FAILURE);//error

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;//UDP socket
   
    n = getaddrinfo(ip_address,port,&hints,&res);
    if(n != 0)/*error*/
        exit(EXIT_FAILURE); 
    
    n=sendto(fd, line, strlen(line), 0, res->ai_addr, res->ai_addrlen);
    if(n==-1)/*error*/
        exit(EXIT_FAILURE);
    
    freeaddrinfo(res);
    addrlen=sizeof(addr);
    n=recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if(n==-1)/*error*/
        exit(EXIT_FAILURE);
    close(fd);
    res_msg = buffer;
    return res_msg;
}