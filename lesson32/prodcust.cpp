#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unistd.h>

// 定义链表节点
struct Node {
    int num;
    std::shared_ptr<Node> next;  // shared_ptr 进行内存管理，方便自动释放节点
};

// 互斥锁和条件变量
std::mutex mtx;
std::condition_variable cv;

// 链表的头节点
std::shared_ptr<Node> head = nullptr;

// 生产者函数
void producer(int id) {
    for (int i = 0; i < 20; ++i) {
        // 创建新节点
        std::shared_ptr<Node> newNode = std::make_shared<Node>();
        newNode->num = rand() % 1000;

        // 加锁并操作链表
        std::unique_lock<std::mutex> lock(mtx);
        newNode->next = head;
        head = newNode;
        std::cout << "Producer " << id << " produced " << newNode->num << std::endl;

        // 通知消费者有新的节点
        cv.notify_all();
        
        // 解锁后休眠一段时间模拟生产过程
        usleep(100000);
    }
}

// 消费者函数
void consumer(int id) {
    for (int i = 0; i < 20; ++i) {
        std::shared_ptr<Node> tmp;

        {
            std::unique_lock<std::mutex> lock(mtx);
            // 等待直到链表不为空
            cv.wait(lock, [] { return head != nullptr; });

            // 从链表头部删除节点
            tmp = head;
            head = head->next;
            std::cout << "Consumer " << id << " consumed " << tmp->num << std::endl;
        }

        // 休眠一段时间模拟消费过程
        usleep(100000);
    }
}

int main() {
    // 创建两个生产者线程和两个消费者线程
    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
    std::thread c1(consumer, 1);
    std::thread c2(consumer, 2);

    // 等待线程完成
    p1.join();
    p2.join();
    c1.join();
    c2.join();

    return 0;
}
