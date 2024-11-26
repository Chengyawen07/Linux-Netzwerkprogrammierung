#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(){

    // 1. 创建一个通信fd
    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(fd == -1){
        perror("socket");
        exit(-1);
    }

    // 2. bind
    struct sockaddr_in saddr;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);

    int ret = bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(ret == -1){
        perror("bind");
        exit(-1);
    }

    // 3 开始通信
    while(1){

        // 5. 用于数据通信的buf
        char buf[128];
        // 7. 用一个buf存放客户端放ip
        char ipbuf[16];

        // 6. 用于接收客户端的地址 caddr
        struct sockaddr_in caddr;
        int len = sizeof(caddr);

        // 4. recvfrom 接收数据
        // udp里，这个fd可以直接用来数据通信
        int num = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&caddr, &len);  

        // 7. 输出接收到的client的数据，转换他们的ip和port
        printf("client IP: %s, Port : %d\n",
            inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ipbuf, sizeof(ipbuf)),
            ntohs(caddr.sin_port));

        printf("received buf: %s", buf);

        // 8. 再发送原数据回服务端
        sendto(fd, buf, strlen(buf)+1, 0, (struct sockaddr *)&caddr, sizeof(caddr));

    }

    close(fd);

    return 0;
}