#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
 
char* getIPadress()
{
    char hostbuffer[256];
    struct hostent *host_entry;
    int hostname;
    struct in_addr **addr_list;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1) {
        perror("gethostname error");
        exit(1);
    }
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        perror("gethostbyname error");
        exit(1);
    }
    addr_list = (struct in_addr **)host_entry->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++) {
        return inet_ntoa(*addr_list[i]);
    }
}

int main(void)
{
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    int fd,errcode;
    ssize_t n;
    char buffer[128];
    fd=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
    if(fd==-1)/*error*/
        exit(1);

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;//UDP socket
    hints.ai_flags = AI_PASSIVE;

    errcode=getaddrinfo(NULL,"58001",&hints,&res);
    if(errcode!=0)/*error*/
        exit(1);
    
    n = bind(fd,res->ai_addr, res->ai_addrlen);
    if(n==-1)/*error*/
        exit(1);

    while (1){
        addrlen=sizeof(addr);
        n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
        if(n==-1)/*error*/
            exit(1);
        
        write(1,"received: ",10);//stdout
        write(1,buffer,n);
        n=sendto(fd,buffer,n,0,(struct sockaddr*)&addr,addrlen);
        if(n==-1)/*error*/
            exit(1);
    }
    freeaddrinfo(res);
    exit(0);
}
