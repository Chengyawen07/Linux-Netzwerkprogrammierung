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




// main的核心是   Proactor模式
// 主线程把数据读出来。然后任务类，然后工作线程在线程池里，在线程池里取出一个线程来操作
int main(int argc, char * argv[]){

    if(argc <= 1) {
        printf("port number :");
        exit(-1);
    }

    // 服务器从命令行参数获取端口号（如 8080）
    int port = atoi(argv[1]); 

    // 对SIGPIE做处理：忽略 SIGPIPE 信号，防止客户端异常断开时程序崩溃
    addsig(SIGPIPE, SIG_IGN);

    // 初始化线程池
    // 创建一个线程池对象，用于处理客户端任务。
    threadpool<http_conn> * pool = NULL;
    try{
        pool = new threadpool<http_conn>;
    } catch(...){
        exit(-1);
    }

    // 创建数组,保存所有的client连接信息
    // http_conn 是自定义的连接类，每个对象表示一个客户端连接
    http_conn * users = new http_conn[MAX_FD];
    
    // ------- 网络部分的代码 -----------
    // 创建一个 TCP 套接字，用于监听客户端连接请求。
    int lfd = socket(PF_INET, SOCK_STREAM, 0);

    // bind之前 设置端口复用 (但凡要设置端口复用，都是这个代码)
    int reuse = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 配置服务器地址
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);  // 这里的port是前面获取到的
    saddr.sin_addr.s_addr = INADDR_ANY;


    // bind到自己的服务器地址
    bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    // 开始监听客户端连接请求，允许最多 5 个未处理连接排队
    listen(lfd, 5);

    // 创建epoll对象, 写入事件数组，添加fd
    epoll_event events[MAX_EVENT_NUM];

    // 创建epoll对象：epollfd 用于管理事件
    int epollfd = epoll_create(5);

    // 将监听套接字（lfd）添加到 epoll 中
    addfd(epollfd, lfd, false);
    // 静态成员变量 http_conn::m_epollfd 保存 epoll 实例，供所有客户端共享（静态变量是共享的）
    http_conn::m_epollfd = epollfd; 


    // 6. 主事件循环
    while(true){
        // 调用 epoll_wait 等待事件触发：
        // num 是触发的事件数量，events 保存这些事件的信息
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);  
        if((num < 0) && (errno != EINTR)){
            printf("epoll failure \n");
            break;
        }

        // 遍历触发的事件，处理每个文件描述符（fd）上的事件
        for(int i=0; i<num; i++){
            // sockfd就是被触发的事件fd
            int sockfd = events[i].data.fd;

            // 1. 处理监听套接字（新连接）
            // 如果触发事件的文件描述符是监听套接字 lfd，就accept
            if(sockfd == lfd){
                // client
                struct sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                // connect fd
                int  cfd = accept(lfd, (struct sockaddr *)&client_addr, &client_addrlen);

                // 1-1 如果连接已达上限（MAX_FD），拒绝连接并关闭套接字
                if(http_conn::m_user_count >= MAX_FD){
                    // 目前连接满了
                    // 给客户端返回一个 服务器正忙（涉及到相应报文）
                    close(cfd);
                    continue;
                }

                // 1-2 否则，初始化客户端连接信息（users[cfd].init）
                users[cfd].init(cfd, client_addr);

            }  // 2. 处理异常事件: 如果不是lfd, 即如果客户端异常断开或发生错误，关闭连接。
            // 如果读取成功，将任务添加到线程池处理。 如果读取失败，关闭连接
            else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                users[sockfd].close_conn();

            } // 3.处理读事件：如果有可读事件（EPOLLIN）, 调用read
            else if (events[i].events && EPOLLIN) {
                // 判断是否有 读事件 发生
                if(users[sockfd].read()){
                    // read 一次性把数据都读完
                    pool->append(users + sockfd);
                } else {
                    // 如果没有读事件，或读失败，那就关闭连接
                    users[sockfd].close_conn();
                }

            } // 4. 处理写事件：如果有可写事件（EPOLLOUT）,调用write
            // 如果写失败，就关闭连接
            else if(events[i].events & EPOLLOUT) {
                // 如果有写事件：补充：users 是一个动态分配的数组，数组的类型是 http_conn 类
                // users[sockfd].write() 调用的是 http_conn 类的 write() 方法：表示http_conn::write() 的实现逻辑
                // bool http_conn::write() {...}; 如果 write() 返回 false，说明数据未能完全写完，需要关闭连接。
                if (!users[sockfd].write()){  // 一次性写完数据
                    users[sockfd].close_conn();
                }
            }
            

        }
    }

    // 7. 资源清理：关闭 epoll 和监听套接字
    close(epollfd);
    close(lfd);

    delete [] users;
    delete pool;

    return 0;
}