#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {    

    printf("hello, world\n");

    int num = 10;

    // 创建子进程
    pid_t pid = fork();

    // 判断是父进程还是子进程
    if(pid > 0) {
        printf("pid : %d\n", pid);
        // 如果大于0，返回的是创建的子进程的进程号，当前是父进程
        printf("父进程, pid : %d, ppid : %d\n", getpid(), getppid());

        // for循环
        for(int i = 0; i < 8; i++) {
            printf("i : %d , pid : %d\n", i , getpid());
            sleep(1);
        }

    } else if(pid == 0) {
        // 当前是子进程
        printf("子进程, pid : %d, ppid : %d\n", getpid(),getppid());

        // for循环
        for(int j = 0; j < 8; j++) {
            printf("j : %d , pid : %d\n", j , getpid());
            sleep(1);
        }
    }


    return 0;
}