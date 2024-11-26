#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {

    // 1. 创建socket
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket");
        return -1;
    }

    // 2. 设置服务器地址
    struct sockaddr_in seraddr;
    inet_pton(AF_INET, "127.0.0.1", &seraddr.sin_addr.s_addr);
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(9999);

    // 3. 连接到服务器
    int ret = connect(fd, (struct sockaddr *)&seraddr, sizeof(seraddr));

    if(ret == -1){
        perror("connect");
        return -1;
    }

    // 4. 主循环：发送和接收数据
    int num = 0;

    while(1) {

        // 使用 sprintf 将数据和计数器 num 格式化为字符串。
        char sendBuf[1024] = {0};
        // sprintf(sendBuf, "send data %d", num++);  

        fgets(sendBuf, sizeof(sendBuf), stdin);  // 用于从 输入流 中读取字符串
        
        // 使用 write 将字符串发送到服务器。
        write(fd, sendBuf, strlen(sendBuf) + 1);

        // 接收数据
        // 使用 read 函数从服务器接收数据，存入 sendBuf。
        int len = read(fd, sendBuf, sizeof(sendBuf));
        if(len == -1) {
            perror("read");
            return -1;
        }else if(len > 0) {
            printf("read buf = %s\n", sendBuf);
        } else {
            printf("服务器已经断开连接...\n");
            break;
        }
        sleep(1);
        // 延迟 1 毫秒，防止发送数据过快。
        // usleep(1000);
    }

    close(fd);

    return 0;
}
