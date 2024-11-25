#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

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

int main(void){
    struct addrinfo hints,*res;
    int fd,n;
    ssize_t nbytes,nleft,nwritten,nread;
    struct sigaction act;
    char *ptr,buffer[128];
    
    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE,&act,NULL)==-1)/*error*/
        exit(1);

    fd=socket(AF_INET,SOCK_STREAM,0);//TCP socket

    if(fd==-1)
        exit(1);//error

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_STREAM;//TCP socket

    n=getaddrinfo(getIPadress(),"58001",&hints,&res);
    if(n!=0)/*error*/
        exit(1);

    n=connect(fd,res->ai_addr,res->ai_addrlen);
    if(n==-1)/*error*/
        exit(1);

    ptr=strcpy(buffer,"Hello!\n");
    nbytes=7;
    nleft=nbytes;

    while(nleft>0){
        nwritten=write(fd,ptr,nleft);
        if(nwritten<=0)/*error*/
            exit(1);
        nleft-=nwritten;
        ptr+=nwritten;
    }

    nleft=nbytes; ptr=buffer;
    
    while(nleft>0){
        nread=read(fd,ptr,nleft);
    
        if(nread==-1)/*error*/
            exit(1);
        else if(nread==0)
            break;//closed by peer
        nleft-=nread;
        ptr+=nread;
    }
    nread=nbytes-nleft;
    close(fd);
    write(1,"echo: ",6);//stdout
    write(1,buffer,nread);
    exit(0);

}