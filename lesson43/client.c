// TCP通信的客户端

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {

    // 1.创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(-1);
    }

    // 2.连接服务器端(自己链接自己的端口和IP)
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if(ret == -1) {
        perror("connect");
        exit(-1);
    }

    // 3. 通信
    char recvBuf[1024] = {0};
    int i = 0;

    while(1) {

        // char * data = "hello,i am client";

        // 在这里，sprintf 的作用是将格式化的字符串写入到 recvBuf 缓冲区中。
        // 这个函数的功能类似于 printf，但不是直接输出到屏幕，而是将结果存储在指定的字符串缓冲区中。
        sprintf(recvBuf, "data : %d\n", i++);

        // 给服务端发送数据
        write(fd, recvBuf , strlen(recvBuf) + 1);

        sleep(1);
        
        int len = read(fd, recvBuf, sizeof(recvBuf));

        if(len == -1) {
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

    // 关闭连接
    close(fd);

    return 0;
}