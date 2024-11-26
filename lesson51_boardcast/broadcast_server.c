#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0); // 创建UDP套接字
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置广播属性
    int broadcast = 1;
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // 设置广播地址
    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(9999);
    inet_pton(AF_INET, "10.9.31.255", &broadcast_addr.sin_addr.s_addr);

    // 循环发送广播消息
    while (1) {
        char message[] = "Hello, this is a broadcast!";
        sendto(fd, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        printf("Broadcast sent: %s\n", message);
        sleep(1); // 每秒发送一次
    }

    close(fd);
    return 0;
}
