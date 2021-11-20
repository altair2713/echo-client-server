#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <errno.h>
#define SA struct sockaddr
void client_thread(int fd)
{
    char buffer[BUFSIZ];
    while(1) {
        int len=recv(fd,buffer,BUFSIZ-1,0);
        if(len<=0) {
            if(!errno) printf("Connection terminated from server\n");
            else printf("Failed at receive!\n");
            break;
        }
        buffer[len]='\0';
        printf("%s\n",buffer);
    }
    close(fd);
    exit(0);
}
int main(int argc, char **argv)
{
    if(argc!=3) {
        printf("syntax : echo-client <ip> <port>");
        printf("sample : echo-client 192.168.10.2 1234");
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    if(!inet_aton(argv[1],&addr.sin_addr)) {
        printf("Failed to get valid IP Adress\n");
        return -1;
    }
    addr.sin_family=AF_INET;
    addr.sin_port=htons(atoi(argv[2]));
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0) {
        printf("Failed at socket creation\n");
        return -1;
    }
    if(connect(sockfd,(SA*)&addr,sizeof(addr))<0) {
        printf("Failed at connection\n");
        close(sockfd);
        return -1;
    }
    std::thread thread(client_thread,sockfd);
    thread.detach();
    while(1) {
        char buffer[BUFSIZ];
        fgets(buffer,BUFSIZ,stdin);
        if(send(sockfd,buffer,strlen(buffer),0)<=0) {
            printf("Failed at send!\n");
            break;
        }
    }
    close(sockfd);
    return 0;
}
