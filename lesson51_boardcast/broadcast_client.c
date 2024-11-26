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

    // 绑定本地地址和端口
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(9999);
    local_addr.sin_addr.s_addr = INADDR_ANY; // 接收任意地址的广播

    if (bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(fd);
        exit(1);
    }

    // 循环接收广播消息
    while (1) {
        char buf[128];
        int len = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
        if (len > 0) {
            buf[len] = '\0';
            printf("Received broadcast: %s\n", buf);
        }
    }

    close(fd);
    return 0;
}
