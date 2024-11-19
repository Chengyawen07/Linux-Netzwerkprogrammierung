#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int counter = 0;  // 共享资源
pthread_rwlock_t rwlock;  // 读写锁

void* read_counter(void* arg) {
  
    for (int i = 0; i < 5; i++) {
      
        pthread_rwlock_rdlock(&rwlock);  // 加读锁
        printf("Reader %ld reads counter = %d\n", pthread_self(), counter);
        pthread_rwlock_unlock(&rwlock);  // 解锁
        usleep(100000);  // 模拟读取的时间延迟
    }
    return NULL;
}

void* write_counter(void* arg) {
  
    for (int i = 0; i < 5; i++) {
      
        pthread_rwlock_wrlock(&rwlock);  // 加写锁
        counter++;  // 修改共享资源
        printf("Writer %ld updates counter to %d\n", pthread_self(), counter);
        pthread_rwlock_unlock(&rwlock);  // 解锁
        usleep(200000);  // 模拟写入的时间延迟
    }
    return NULL;
}



int main() {
    pthread_t readers[3], writer;
    pthread_rwlock_init(&rwlock, NULL);  // 初始化读写锁

    // 创建3个读线程
    for (int i = 0; i < 3; i++) {
        pthread_create(&readers[i], NULL, read_counter, NULL);
    }
    
    // 创建1个写线程
    pthread_create(&writer, NULL, write_counter, NULL);

    // 等待所有线程完成
    for (int i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(writer, NULL);

    pthread_rwlock_destroy(&rwlock);  // 销毁读写锁
    return 0;
}
