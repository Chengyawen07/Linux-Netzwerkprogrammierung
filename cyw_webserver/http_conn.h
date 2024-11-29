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


class http_conn {

public:

    // 所有Socket上的事件，都被注册到epollfd上
    static int m_epollfd; 
    // 统计用户数量
    static int m_user_count;

    // 1. 构造
    http_conn(){}
    ~http_conn(){}

    // process: 用来处理客户端的请求
    void process();  

    void init(int sockfd, const sockaddr_in & addr);

private:
    int m_sockfd;  // 该http连接的socket
    sockaddr_in m_address; // socket通信地址
    

};



#endif