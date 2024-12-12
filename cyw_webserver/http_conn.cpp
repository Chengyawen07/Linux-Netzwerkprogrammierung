#include "http_conn.h"
#include <string.h>


// 给共享的static epollfd 和 user_count 初始化
int http_conn::m_epollfd = -1;
int http_conn::m_user_count = 0;

// 设置文件描述符非阻塞
void setnonblocking(int fd){
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
}


// main.cpp
// 添加文件描述符fd到epoll中： 这两个函数在外部实现，在http_conn.cpp里实现
void addfd(int epollfd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP; // LT 水平触发, RDUP: 这样就可以通过事件来判断
    // event.events = EPOLLIN | EPOLLET; // ET 边缘触发

    // epoll one shot: 只能触发一次，防止同一通信被不同线程处理
    if(one_shot) {
        event.events | EPOLLONESHOT; 
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

    // 设置文件描述符为非阻塞
    setnonblocking(fd);
}

// main.cpp
// 从epoll中删除文件描述符fd
void removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

// main.cpp
// 修改文件描述符，重置Socket上的epolloneshot事件。
// 确保下一次文件可读时， 事件能被触发，能有提示
void modifyfd(int epollfd, int fd, int ev) {
    epoll_event event;
    // 设置 event 的 data.fd，表示与这个事件关联的文件描述符是 fd
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;  // 设置时间为one shot, 和查询客户端断开
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);  // 用来操作 epoll 实例, 用来modify fd
}

// http_conn类内函数：初始化连接client connection
void http_conn::init(int sockfd, const sockaddr_in & addr) {
    m_sockfd = sockfd;
    m_address = addr;

    // 设置端口复用
    int reuse = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 添加到epoll对象中
    addfd(m_epollfd, sockfd, true);
    m_user_count++; // 总用户数+1

}


// 这个也是 http_conn类内函数: 关闭连接
void http_conn::close_conn(){
    if(m_sockfd != -1){
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--; // 关闭连接，客户端数量-1
    }

}

// 循环读取客户数据，直到无数据可读，或对方关闭连接
bool http_conn::read(){
    printf("一次性读完所有数据\n");
    if(m_read_idx >= READ_BUFFER_SIZE) {
        return false;
    }

    // 这次读取到的字节
    int bytes_read = 0;
    while(true) {
        // m_read_buf 数据已读取的位置
        // m_read_idx 偏移量：当前缓冲区中有效数据的索引
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        if(bytes_read == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false;
        } else if(bytes_read == 0){
            // ==0, 表示对方关闭连接（也就是没有读到数据）
            return false;
        } else {
            // 成功读取数据的时候：将读取的字节数累加到 m_read_idx
            m_read_idx += bytes_read;
        }
    }

    return true;
}


bool http_conn::write(){
    printf("一次性写完所有数据\n");
    return true;
}

 

// 用有限状态机来解析请求
//解析HTTP请求
http_conn::HTTP_CODE http_conn::process_read(){
    return http_conn::NO_REQUEST; // 需要使用作用域限定符

}
 // 解析请求首行
http_conn::HTTP_CODE http_conn::parse_request_line(char * text){
    return http_conn::NO_REQUEST; // 需要使用作用域限定符

}
//解析请求头
http_conn::HTTP_CODE http_conn::parse_headers(char * text){
    return http_conn::NO_REQUEST; // 需要使用作用域限定符

}
// 解析请求体
http_conn::HTTP_CODE http_conn::parse_content(char * text){
    return http_conn::NO_REQUEST; // 需要使用作用域限定符

}

http_conn::LINE_STATUS parse_line(); 





// 业务处理
// 这个process是由线程池中的工作线程调用的 操作函数
void http_conn::process() {
    // 定义完整的 HTML 响应正文
    printf("parse request,create response\n");

    // 解析HTTP请求
    HTTP_CODE read_ret =process_read();
    if(read_ret == NO_REQUEST){
        modifyfd(m_epollfd, m_sockfd, EPOLLIN);
        return;
    }

    // 生成相应
    // bool write_ret = process_write();


}

 

