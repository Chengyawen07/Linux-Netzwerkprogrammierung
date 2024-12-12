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

    // 有限状态机的状态：
    // HTTP请求方法，这里只支持GET
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        解析客户端请求时，主状态机的状态
        CHECK_STATE_REQUESTLINE:当前正在分析请求行
        CHECK_STATE_HEADER:当前正在分析头部字段
        CHECK_STATE_CONTENT:当前正在解析请求体
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    
    /*
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST          :   请求不完整，需要继续读取客户数据
        GET_REQUEST         :   表示获得了一个完成的客户请求
        BAD_REQUEST         :   表示客户请求语法错误
        NO_RESOURCE         :   表示服务器没有资源
        FORBIDDEN_REQUEST   :   表示客户对资源没有足够的访问权限
        FILE_REQUEST        :   文件请求,获取文件成功
        INTERNAL_ERROR      :   表示服务器内部错误
        CLOSED_CONNECTION   :   表示客户端已经关闭连接了
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    // 从状态机的三种可能状态，即行的读取状态，分别表示
    // 1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

    // ----------------------- 状态机结束 -----------------

    // 1. 构造
    http_conn(){}
    ~http_conn(){}

    // process: 用来处理客户端的请求
    void process();  

    void init(int sockfd, const sockaddr_in & addr);
    void close_conn();  // 关闭连接
    bool read();  // 非阻塞的读（当检测到触发的fd的读，就调用read）
    bool write(); // 非阻塞的写

    // 用有限状态机来解析请求
    HTTP_CODE process_read(); //解析HTTP请求
    HTTP_CODE parse_request_line(char * text); // 解析请求首行
    HTTP_CODE parse_headers(char * text); //解析请求头
    HTTP_CODE parse_content(char * text); // 解析请求体

    LINE_STATUS parse_line(); 


    

private:
    int m_sockfd;  // 该http连接的socket
    sockaddr_in m_address; // socket通信地址

    // 为了read 函数，定义的变量
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx; // 表示读buffer中，目前读到的位置index

    // char m_write_buf[];

    

};



#endif