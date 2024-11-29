#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "locker.h"
#include "thradpool.h"
#include <signal.h>
#include "http_conn.h"

#define MAX_FD 65535 // max clients
#define MAX_EVENT_NUM 10000  // 一次监听的最大的数量


// add：添加信号捕捉
void addsig(int sig, void(handler)(int)) {
    struct sigaction sa;                      // 定义 sigaction 结构体
    memset(&sa, '\0', sizeof(sa));            // 清空结构体，确保字段初始化为 0
    sa.sa_handler = handler;                  // 设置信号处理函数
    sigfillset(&sa.sa_mask);                  // 屏蔽所有其他信号
    // assert(sigaction(sig, &sa, NULL) != -1);  // 注册信号处理函数并验证成功
}



// 添加文件描述符到epoll中： 这两个函数在外部实现，在http_conn.cpp里实现
extern void addfd(int epollfd, int fd, bool one_shot);
// 从epoll中删除文件描述符
extern void removefd(int epollfd, int fd);

// 修改
extern void modifyfd(int epollfd, int fd, int ev);


int main(int argc, char * argv[]){

    if(argc <= 1) {
        printf("port number :");
        exit(-1);
    }

    // 获取端口号
    int port = atoi(argv[1]); 

    // 对SIGPIE做处理
    addsig(SIGPIPE, SIG_IGN);

    // 初始化线程池
    threadpool<http_conn> * pool = NULL;
    try{
        pool = new threadpool<http_conn>;
    } catch(...){
        exit(-1);
    }

    // 创建数组,保存所有的client连接信息
    http_conn * users = new http_conn[MAX_FD];
    
    // 网络代码:
    // 创建Socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);

    // bind之前设置端口复用 (但凡要设置端口复用，都是这个代码)
    int reuse = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);  // 这里的port是前面获取到的
    saddr.sin_addr.s_addr = INADDR_ANY;


    // bind
    bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    // 监听
    listen(lfd, 5);

    // 创建epoll对象, 写入事件数组，添加fd
    epoll_event events[MAX_EVENT_NUM];

    // 创建epoll对象
    int epollfd = epoll_create(5);

    // 将监听的文件描述符，添加到epoll
    addfd(epollfd, lfd, false);
    http_conn::m_epollfd = epollfd; // 静态成员设置成epollfd

    while(true){
        // 循环检测是否有事件发生：
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);  // 返回触发的事件个数
        if((num < 0) && (errno != EINTR)){
            printf("epoll failure \n");
            break;
        }

        // 循环遍历数组 accept client
        for(int i=0; i<num; i++){
            int sockfd = events[i].data.fd;
            // 如果是listen fd
            if(sockfd == lfd){
                // client
                struct sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                // connect fd
                int  cfd = accept(lfd, (struct sockaddr *)&client_addr, &client_addrlen);

                if(http_conn::m_user_count >= MAX_FD){
                    // 目前连接满了
                    // 给客户端返回一个 服务器正忙（涉及到相应报文）
                    close(cfd);
                    continue;
                }

                // 将新的client初始化，放到数组中
                users[cfd].init(cfd, client_addr);

            }  else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                // 如果不是lfd，对方异常断开或错误等事件，则关闭连接
                users[sockfd].close_conn();

            } else if (events[i].events && EPOLLIN) {
                // 判断是否有 读事件 发生
                if(users[sockfd].read()){
                    // read 一次性把数据都读完
                    pool->append(users + sockfd);
                } else {
                    // 如果没有读事件，那就关闭
                    users[sockfd].close_conn();
                }

            }
            
            



        }
    }












    return 0;
}