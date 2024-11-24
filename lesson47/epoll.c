#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>


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

    // 4. 创建一个 epoll
    int epfd = epoll_create(100);

    // 5. 操作epoll，将监听lfd加入到epoll, 添加到内核红黑树里
    // epev 是单个结构体：	用于向 epoll_ctl 注册事件
    // 需要手动赋值，因为程序需要明确告诉内核监听的事件epollin和文件描述符lfd。
    struct epoll_event epev;
    epev.events = EPOLLIN; // 检测读事件
    epev.data.fd = lfd; // 只使用.data里的fd，其他参数不用管
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &epev);


    // 输出参数：检测到的要处理的数组 epevs：
        // wait第二个参数 epevs，用来接受检测到的fd! 不需要提前初始化
        // 结构体数组：是声明了一个包含 1024 个 struct epoll_event 类型元素的数组。
    struct epoll_event epevs[1024];

    
    // while
    while(1){

        // wait来等待有没有改变的fd
        // 第二个epevs参数虽然是指针，但是epevs是数组，不用再加&了！
        int ret = epoll_wait(epfd, epevs, 1024, -1); // -1表示一直阻塞，等待事件直到发生
        if(ret == -1) {
            perror("epoll");
            exit(-1);
        }

        printf("ret == %d\n", ret);   //  成功，返回发送变化的文件描述符的个数; 失败返回 -1


        // for 循环遍历， ret 是返回到的已经发生改变的 数组epevs.data.fd
        for(int i=0; i<ret; i++){

            // 5.定义curfd代替epevs返回值，接收到的当前要处理的fd
            int curfd = epevs[i].data.fd; 

            if(curfd == lfd) {
                // 1. 如果epoll监听的文件有lfd客户端连接, 就accept
                // 2. 为了接收client的信息，你要创建一个caddr
                struct sockaddr_in caddr;
                int len = sizeof(caddr);
                int cfd = accept(lfd, (struct sockaddr *)&caddr, &len);

                // 3. 把通信cfd追加到红黑树epev里 (和之前新增lfd方法一样)
                epev.events = EPOLLIN;
                epev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &epev);

            } else {
                // 4. else说明有数据到达：读数据。数据已经到达epevs
                char buf[1024] = {0};
                int len = read(curfd, buf, sizeof(buf));

                if(len == -1) {
                    perror("read");
                    exit(-1);
                } else if(len == 0) {  // 客户端关闭：当 read 返回 0 时，表示客户端已经断开连接
                    printf("client closed...\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                    close(curfd);   // 关闭文件描述符
                } else if(len > 0) {
                    printf("read buf = %s\n", buf);
                    write(curfd, buf, strlen(buf) + 1);  // 重新写给客户端，要write的文件描述符是curfd！
                }
            }
        }

    }

    close(lfd);
    close(epfd);  // create 创建的epfd，使用完要关闭

    return 0;
}


