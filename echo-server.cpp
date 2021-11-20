#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <set>
#include <thread>
#include <mutex>
#include <errno.h>
#define SA struct sockaddr
bool echo,broadcast;
std::set<int> client_fd;
std::mutex mutex;
void server_thread(int fd)
{
    char buffer[BUFSIZ];
    while(1) {
        int len=recv(fd,buffer,BUFSIZ-1,0);
        if(len<=0) {
            if(!errno) printf("Connection terminated from client number %d\n",fd);
            else printf("Failed at receive!\n");
            break;
        }
        buffer[len]='\0';
        printf("%s\n",buffer);
        if(echo) {
            if(broadcast) {
                mutex.lock();
                for(auto it = client_fd.begin(); it != client_fd.end(); it++) {
                    if(send(*it,buffer,len,0)<=0) {
                        printf("Failed at send!\n");
                        client_fd.erase(it);
                        it--;
                    }
                }
                mutex.unlock();
            }
            else {
                if(send(fd,buffer,len,0)<=0) {
                    printf("Failed at send!\n");
                    break;
                }
            }
        }
    }
    if(broadcast) {
        mutex.lock();
        client_fd.erase(fd);
        mutex.unlock();
    }
    close(fd);
    return;
}
int main(int argc, char **argv)
{
    if(argc<2||argc>4) {
        printf("echo-server <port> [-e[-b]]\n");
        printf("echo-server 1234 -e -b\n");
        return -1;
    }
    if(argc>=3) {
        if(strncmp(argv[2],"-e",strlen(argv[2]))) {
            printf("Unavailable option\n");
            return -1;
        }
        echo=1;
    }
    if(argc==4) {
        if(strncmp(argv[3],"-b",strlen(argv[3]))) {
            printf("Unavailable option\n");
            return -1;
        }
        broadcast=1;
    }
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0) {
        printf("Failed at socket creation\n");
        return -1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(atoi(argv[1]));
    if(bind(sockfd,(SA*)&server_addr,sizeof(server_addr))) {
        printf("Failed at binding\n");
        close(sockfd);
        return -1;
    }
    if(listen(sockfd,SOMAXCONN)) {
        printf("Failed at Listen\n");
        close(sockfd);
        return -1;
    }
    while(1) {
        SA client_addr;
        socklen_t client_len=sizeof(client_addr);
        int client_sockfd=accept(sockfd,(SA*)&client_addr,&client_len);
        if(client_sockfd<0) {
            printf("Failed to accept for client\n");
            return -1;
        }
        if(broadcast) {
            mutex.lock();
            client_fd.insert(client_sockfd);
            mutex.unlock();
        }
        std::thread thread(server_thread,client_sockfd);
        thread.detach();
    }
    close(sockfd);
    return 0;
}
