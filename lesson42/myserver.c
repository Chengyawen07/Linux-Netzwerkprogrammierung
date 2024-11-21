
// TCP server

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>     // 包含了这个头文件，上面两个就可以省略
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {

    // 1. create
    int lfd = socket(AF_INET, SOCK_STREAM, 0); // TCP 协议

    // 判断
    if(lfd == -1){
        perror("socket");
        exit(-1);
    }
    
    // 2 bind
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    // inet_pton(AF_INET, "192.168.", saddr.sin_addr.s_addr);
    saddr.sin_addr.s_addr = INADDR_ANY; // 表示任意地址 0.0.0.0
    saddr.sin_port = htons(9999); // port

    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret == -1){
        perror("bind");
        exit(-1);
    }


    // 3 listen
    ret = listen(lfd, 8);

    if(ret == -1){
        perror("listen");
        exit(-1);
    }

    // 4 accept
    struct sockaddr_in clientaddr;
    int len = sizeof(clientaddr);
    int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);

    if(cfd == -1){
        perror("accept");
        exit(-1);
    }

    // 5 输出客户端信息
    char clientIP[16];

    // 转换IP
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    uint16_t clientPort = ntohs(clientaddr.sin_port); // 客户端的IP，从网络n转到p
    printf("client ip = %s, port is %d\n", clientIP, clientPort);


    // 6. 从客户端接收数据 recv
    // 用 = {0} 来初始化数组为全 0 是一种安全、方便的做法，
    // 尤其是在处理字符串或数据缓冲区时，避免了不必要的麻烦。
    char recvBuf[1024] = {0};

    // C 语言不允许在同一个作用域中定义相同名字的变量。
    // ※ 前面有了int len，后面就不能再出现int len了。要把int删掉！
    //  或者这里直接就用num 
    int num = read(cfd, recvBuf, sizeof(recvBuf));
    if(num == -1) {
        perror("read");
        exit(-1);
    } else if(num > 0) {
        printf("recv client data : %s\n", recvBuf);
    } else if(num == 0) {
        // 表示客户端断开连接
        printf("clinet closed...");
    }

    // 给客户端发数据
    char * data = "hello I am server";
    write(cfd, &data, strlen(data));

    // 关闭文件描述符
    close(cfd);
    close(lfd);

    return 0;
}