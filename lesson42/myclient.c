
// TCP通信的客户端

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>



int main(){

    // 1. create
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(-1);
    }

    // 2 connect (连接到Server的addr)
    struct sockaddr_in serveraddr;
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9999);
    
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if(ret == -1) {
        perror("connect");
        exit(-1);
    }

    // 3 通信 客户端要给Server发消息
    char recvBuf[1024] = {0};

    char * data = "hello, I am client";

    write(fd, data, strlen(data));

    int len = read(fd, recvBuf, sizeof(recvBuf));
    if(len == -1) {
        perror("read");
        exit(-1);
    } else if(len > 0) {
        printf("recv server data : %s\n", recvBuf);
    } else if(len == 0) {
        // 表示服务器端断开连接
        printf("server closed...");
        // break;
    }

    close(fd);

    return 0;
}
