#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(){

    // 1. socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        perror("socket");
        exit(-1);
    }

    // 2. connect
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;

    // 转换服务器的ip
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);
    // 转换port
    saddr.sin_port = ntohs(9999);

    int ret = connect(fd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(ret == -1){
        perror("connect");
        exit(-1);
    }

    // 3. 通信
    char recvBuf[1024] = {0};
    while(1){

        // 客户端给Server写数据
        write(fd, recvBuf, strlen(recvBuf));

        sleep(1);

        // 读服务端的数据
        int len = read(fd, recvBuf, sizeof(recvBuf));
        if(len == -1){
            perror("read");
            exit(-1);
        } else if(len > 0) {
            printf("recv server data : %s\n", recvBuf);
        } else if(len == 0) {
            // 表示服务器端断开连接
            printf("server closed...");
            break;
        }
    }

    close(fd);

    return 0;
}
