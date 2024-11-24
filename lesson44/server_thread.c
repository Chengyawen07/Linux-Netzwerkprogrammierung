#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include<string.h>

#include <pthread.h>


// 定义Working的传入参数的Struct
struct sockArg {
    int fd;
    pthread_t tid;
    struct sockaddr_in addr;
}

struct sockArg sockinfos[128];



// 子线程函数: 实现客户端和服务端通信
void * working(void * arg){
    // 1. 要得到cfd
    // 2. 要客户端信息caddr

}



// multi-thread
int main(){

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

    // accept, 循环等待客户端连接
    // 一旦有一个客户端，就创建一个子线程 实现通信
    while(1){

        // 保存接收到的client的信息
        struct sockaddr_in caddr;
        int len = sizeof(caddr);
        int cfd = accept(lfd, (struct sockaddr *)&caddr, &len);
        
        // crreate a 子线程
        struct sockArg threadArg;  // 这是一个局部变量， 运行完就失效了，那Working就不行了 X
        
        
        threadArg.fd = cfd;

        pthread_create(&threadArg.tid, NULL, working, NULL);






    }






    return 0;
}