#ifndef HTTPCONNECTION_H
#define HTTPCPNNECTION_H

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdarg.h>

#include "locker.h"

// 这个类主要就是来创建client类型的，包括client会涉及到的函数
// 比如，client初始化init(), 关闭连接clode()
class http_conn {

public:

    // 所有Socket上的事件，都被注册到epollfd上
    static int m_epollfd; 
    // 统计用户数量
    static int m_user_count;

    // 为了read和write函数，定义一个读和写的buffer大小 （静态常量成员变量）
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;


    // 1. 构造
    http_conn(){}
    ~http_conn(){}

    // process: 用来处理客户端的请求
    void process();  

    void init(int sockfd, const sockaddr_in & addr);

    void close_conn();

    // 非阻塞的读（当检测到触发的fd的读，就调用read）
    bool read();

    // 非阻塞的写
    bool write();
    

private:
    int m_sockfd;  // 该http连接的socket
    sockaddr_in m_address; // socket通信地址

    // 为了read 函数，定义的变量
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx; // 表示读buffer中，目前读到的位置index

    // char m_write_buf[];

    

};



#endif