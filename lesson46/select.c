#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>


int main(){
    
    // 1. 创建Socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);

    // 2. 绑定自己的服务器端口
    struct sockaddr_in saddr;
    saddr.sin_port = htons(9999);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    // 3. 监听客户端
    listen(lfd, 8);

    // 4. 创建一个fd set，用来存储 n个客户端的fd
    fd_set rdset, tmp; // 它能表示1024个fd
    FD_ZERO(&rdset);
    FD_SET(lfd, &rdset);
    int maxfd = lfd;

    // 5. 使用Select函数，检测哪些fd可用
    while(1){

        tmp = rdset; // tmp只有自己操作，没有被内核修改

        // 调用select系统函数，让内核帮检测哪些文件描述符有数据
        int ret = select(maxfd+1, &tmp, NULL, NULL, NULL);

        if(ret == -1) {
            perror("select");
            exit(-1);
        } else if(ret == 0) {
            continue;
        } 
        else if(ret > 0) {
            // FD_ISSET : 检查文件描述符 i 是否在 rdset 集合中，并且是否有数据可读.
            // 如果返回 true（非 0），说明该文件描述符有事件需要处
            if(FD_ISSET(lfd, &tmp)) {

                // 表示有新的客户端连接进来了
                struct sockaddr_in cliaddr;
                int len = sizeof(cliaddr);
                int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);

                // 将新的文件描述符加入到集合中! 这里是操作的 内核 rdset
                FD_SET(cfd, &rdset);

                // 更新最大的文件描述符
                maxfd = maxfd > cfd ? maxfd : cfd;
            }

            // 遍历文件描述符: 
            for(int i = lfd + 1; i<= maxfd; i++){
                // 检查 i 是否有数据可读（比如客户端发送了数据）
                if(FD_ISSET(i, &tmp)){

                    // 这个fd对应的客户端发来了数据, 那就读取 read
                    char buf[1024] = {0};
                    int len = read(i, buf, sizeof(buf));

                    if(len == -1) {
                        perror("read");
                        exit(-1);
                    } else if(len == 0) {  // 客户端关闭：当 read 返回 0 时，表示客户端已经断开连接
                        printf("client closed...\n");
                        close(i);   // 关闭文件描述符
                        FD_CLR(i, &rdset);  // 从 rdset 集合中移除该文件描述符
                    } else if(len > 0) {
                        printf("read buf = %s\n", buf);
                        write(i, buf, strlen(buf) + 1);  // 使用 write 函数将数据原样回传给客户端（回显功能）。
                    }
                }
            }



        }
    
    }

    close(lfd);





    return 0;
}