#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

// 线程池类，将它定义为模板类是为了代码复用，模板参数T是任务类
template<typename T>
class threadpool {
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request);


private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void* worker(void* arg);
    void run();

private:
    // 线程的数量
    int m_thread_number;  
    
    // 描述线程池的数组，大小为m_thread_number    
    pthread_t * m_threads;

    // 请求队列中最多允许的、等待处理的请求的数量  
    int m_max_requests; 
    
    // 请求队列
    std::list< T* > m_workqueue;  

    // 保护请求队列的互斥锁
    locker m_queuelocker;   

    // 是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程          
    bool m_stop;                    
};


template< typename T >
threadpool< T >::threadpool(int thread_number, int max_requests) : 
        m_thread_number(thread_number), 
        m_max_requests(max_requests), 
        m_stop(false), 
        m_threads(NULL) {

    // 判断两个传入参数是否正确：
    if((thread_number <= 0) || (max_requests <= 0) ) {
        throw std::exception();
    }

    // 创建一个新线程容器 new，大小就是按照你设置的线程数大小
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads) {
        throw std::exception();
    }

    // 创建多个线程，并将他们设置为脱离线程。
    for ( int i = 0; i < thread_number; ++i ) {
        printf( "create the %dth thread\n", i);

        // 用create创建子线程
        if(pthread_create(m_threads + i, NULL, worker, this ) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
        
        // detach分离：删除数组
        if( pthread_detach( m_threads[i] ) ) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template< typename T >
threadpool< T >::~threadpool() {
    delete [] m_threads;
    m_stop = true;
}


// 这段代码实现了线程池中添加任务到工作队列的逻辑，具体来说是 threadpool 类中 append 方法的实现。
template< typename T >
bool threadpool< T >::append( T* request )
{
    // 操作工作队列时一定要加锁，因为它被所有线程共享。
    m_queuelocker.lock();
    if ( m_workqueue.size() >= m_max_requests ) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template< typename T >
void* threadpool< T >::worker( void* arg )
{
    threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}


template< typename T >
void threadpool< T >::run() {

    while (!m_stop) {
        // 2. 等待信号量通知有任务
        m_queuestat.wait();

        // 3. 加锁以访问任务队列
        // 锁定互斥锁 m_queuelocker，保证对任务队列 m_workqueue 的操作是线程安全的
        m_queuelocker.lock();

        // 4. 检查任务队列是否为空
        if ( m_workqueue.empty() ) {
            m_queuelocker.unlock();
            continue;
        }
        
        // 5. 从任务队列取任务
        T* request = m_workqueue.front();
        m_workqueue.pop_front(); 
        m_queuelocker.unlock();
        if ( !request ) {
            continue;
        }
        request->process();
    }

}

#endif
