

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 全局变量，所有的线程都共享这一份资源。
int tickets = 1000;
pthread_mutex_t mutex;

void * sellticktes(void * arg){

    // x 如果你把变量写到函数里，那就成每个线程都卖了100张门票
    // int tickets = 100;  

    while(1){

        pthread_mutex_lock(&mutex);

        if(tickets > 0){
            usleep(6000);
            printf("当前线程id为%ld, 正在卖第 %d 张门票\n", pthread_self(), tickets);
            tickets--;
        }
        else{
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(){

    // mutex
    pthread_mutex_init(&mutex, NULL);

    // create 3 child pthreads
    pthread_t tid1, tid2, tid3;

    pthread_create(&tid1, NULL, sellticktes, NULL);
    pthread_create(&tid2, NULL, sellticktes, NULL);
    pthread_create(&tid3, NULL, sellticktes, NULL);

    // 设置线程分离
    pthread_detach(tid1);
    pthread_detach(tid2);
    pthread_detach(tid3);

    // 退出主线程
    pthread_exit(NULL);

    // 
    pthread_mutex_destroy(&mutex);


    return 0;
}

