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
    // saddr.sin_addr.s_addr = INADDR_ANY;  // 这个是服务器端的写法
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr); // 在client里，要这么写pton
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);


    // 2. 直接发送消息了
    int num = 0;
    
    while(1){

        char sendBuf[128];
        
        // 用sprintf格式化一个数据，每次发送num++
        sprintf(sendBuf, "hello, I am client %d\n", num++);

        // 给服务端发送数据
        sendto(fd, sendBuf, strlen(sendBuf)+1, 0, (struct sockaddr *)&saddr, sizeof(saddr));


        // 接收数据
        int recvNum = recvfrom(fd, sendBuf, sizeof(sendBuf), 0, NULL, NULL);
        printf("server say: %s\n", sendBuf);

        sleep(1);
    }

    close(fd);

    return 0;
}