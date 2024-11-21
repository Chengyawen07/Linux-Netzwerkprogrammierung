
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/wait.h>
#include<errno.h>


// void recycleChild(int sig) {
//     while (waitpid(-1, NULL, WNOHANG) > 0);
// }

// 信号处理函数，用于回收子进程（捕捉 SIGCHLD 信号）
void recycleChild(int arg) {
    while(1) {
        int ret = waitpid(-1, NULL, WNOHANG);
        if(ret == -1) {
            // 所有的子进程都回收了
            break;
        }else if(ret == 0) {
            // 还有子进程活着
            break;
        } else if(ret > 0){
            // 被回收了
            printf("子进程 %d 被回收了\n", ret);
        }
    }
}


// server
int main(){

    // 为了回收子进程，避免僵尸进程, 我们采用信号捕捉的方式!
    // 我们在while里没办法使用wait来回收，因为wait() 会阻塞主进程：
        // 如果在服务器的主循环中直接调用 wait()，主进程会被阻塞，等待子进程结束后才继续执行。这样，服务器就无法继续处理新的客户端连接
    // 注册 SIGCHLD 信号，用于回收子进程：非阻塞
        // 通过捕捉 SIGCHLD 信号并在信号处理函数中使用 waitpid(-1, NULL, WNOHANG)，可以非阻塞地回收所有已经结束的子进程。
    struct sigaction act;
    act.sa_handler = recycleChild;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);


    // 1.socket
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(lfd == -1) {
        perror("socket");
        exit(-1);
    }


    // 2.b
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret == -1) {
        perror("bind");
        exit(-1);
    }

    // 3. listen
    ret = listen(lfd, 128);

    if(ret == -1) {
        perror("listen");
        exit(-1);
    }

    // 4.accept 阻塞
    while(1){
        
        struct sockaddr_in caddr;
        int len = sizeof(caddr);

        // accept 这里应该是阻塞的
        // cfd = communication fd 
        // 第三个参数： socklen_t *__restrict__ __addr_len 
        int cfd = accept(lfd, (struct sockaddr *)&caddr, &len);
        
        if(cfd == -1) {
            // 这个的作用是，确保如果一个client因为信号终端被回收后，accept还继续保持阻塞状态，而不是exit了！
            if(errno == EINTR){
                continue;  // 继续下一次循环
            }
            perror("accept");
            exit(-1);
        } 

        // 每一个accept连接到client，就创建一个子进程，进行通信
        pid_t pid = fork();

        if(pid == 0){
            // pid==0, child

            // 1. 要通信。第一步先获取客户端信息
            // clientIP用于存储客户端的IP地址字符串。不需要初始化，因为inet_ntop()会直接填充完整的数据。
            char clientIP[16] = {0}; 
            
            // 获取客户端地址IP: clientIP
            // ! inet_ntop() 的第三个参数直接使用了 clientIP，而没有加 &clientIP，这是因为数组名本身在表达式中就会被视为指向第一个元素的指针。
            inet_ntop(AF_INET, &caddr.sin_addr.s_addr, clientIP, sizeof(clientIP));

            // 获取客户端port：ntohs 转换主机字节序
            uint16_t clientPort = ntohs(caddr.sin_port);
            printf("client ip is %s, port is %d\n", clientIP, clientPort);

            // 2 接受客户端发来的数据
            char recvBuf[1024] = {0}; //  读取缓冲区要初始化为0; 他是一个字符数组;
            
            while(1){
                
                // read() 函数会把从客户端接收到的数据写入到 recvBuf 中。
                int read_len = read(cfd, recvBuf, sizeof(recvBuf));
                if(read_len == -1) {
                    perror("read");
                    exit(-1);
                } else if(read_len > 0){
                    printf("recv client data : %s\n", recvBuf);
                } else if(read_len == 0){
                    printf("client closed");
                    break;
                }

                // write：将 recvBuf 中的数据发送回客户端，起到回显的效果。
                // 对于一个字符数组 recvBuf[1024]，sizeof(recvBuf) 总是返回 1024
                // strlen(recvBuf) 可以返回字符串的实际长度。但不包括字符串的 \0。
                // 加 1 的目的是让 write() 把 \0 也发送出去，这样接收端在读取数据时就能正确识别字符串的结束位置。
                write(cfd, recvBuf, strlen(recvBuf) + 1);

            }

            close(cfd);
            exit(0);


            // recvBuf用于接收客户端的数据，它是作为一个缓冲区（buffer）反复使用的。
            // 在这里初始化为{0}，可以确保缓冲区初始为空, 避免读取到旧的或未定义的内容。

        }

    }

    close(lfd);

    return 0;
}