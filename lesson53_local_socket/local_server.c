#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/un.h> // local socket

int main(){

    // 1. creata socket AF_LOCAL 本地套接字
    int lfd = socket(AF_LOCAL, SOCK_STREAM, 0); 
    if(lfd == -1 ){
        perror("socket");
        exit(-1);
    }

    // 2.绑定本地套接字文件
    struct sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, "server.sock");
    int ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1) {
        perror("bind");
        exit(-1);
    }


    // 3. listen
    int ret = listen(lfd, 100);
    if(ret==-1){
        perror("listen");
        exit(-1);
    }


    // 4. wait for clients
    struct sockaddr_un caddr;
    int cli_len = sizeof(caddr);
    int cfd = accept(lfd, (struct sockaddr *)&caddr, &cli_len);
    if ( cfd == -1){
        perror("accept");
        exit(-1);
    }

    printf("client socket filename: %s\n", caddr.sun_path);


    // 5. 通信
    while(1){

        char buf[128];
        int len = recv(cfd, buf, sizeof(buf), 0);

        if(len == -1){
            perror("recv");
            exit(-1);
        } else if(len == 0){
            printf("client closed");
            break;
        } else if(len >0){
            printf("client say: %s\n", buf);
            send(cfd, buf, len, 0);
        }

    }

    close(lfd);
    close(cfd);

    return 0;


}