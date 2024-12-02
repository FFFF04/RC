#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

int main(void){
    struct addrinfo hints,*res;
    int fd, n, errcode, newfd;
    ssize_t nbytes,nleft,nwritten,nread,nw;
    struct sockaddr_in addr;
    socklen_t addrlen;
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
    hints.ai_flags = AI_PASSIVE;
    errcode=getaddrinfo(NULL, "58001", &hints, &res);
    if(errcode!=0)/*error*/
        exit(1);
    if(bind(fd, res->ai_addr, res->ai_addrlen)==-1)/*error*/{
        perror("bind");
        exit(1);
    }
        
    if(listen(fd,5)==-1)/*error*/
        exit(1);

    while(1){
        addrlen=sizeof(addr);
        if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen))==-1)
            exit(1);/*error*/
        while((n=read(newfd,buffer,128))!=0){
            if(n==-1)/*error*/
                exit(1);
            ptr=&buffer[0];
            if(write(1,"received: ",10) <= 0)/*error*/
                exit(1);
            while(n>0){
                if((nw=write(newfd,ptr,n))<=0)/*error*/
                    exit(1);
                if(write(1,buffer,n) <= 0)/*error*/
                    exit(1);
                n-=nw; 
                ptr+=nw;
            }
        }
        close(newfd);
    }
    freeaddrinfo(res);
    close(fd);
    exit(0);
}