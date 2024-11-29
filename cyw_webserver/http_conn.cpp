#include "http_conn.h"


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

    // epoll one shot: 只能触发一次。如果想要再触发，就需要重置
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

