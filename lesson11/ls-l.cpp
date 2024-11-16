#include <iostream>     // 用于 std::cout 和 std::cerr
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <cstring>

// 模拟实现 ls -l 指令
// -rw-rw-r-- 1 nowcoder nowcoder 12 Dec  3 15:48 a.txt

using namespace std;

int main(int argc, char * argv[]) {

    // 判断输入的参数是否正确
    if(argc < 2) {
        // 使用 std::cerr 进行错误输出
        cerr << argv[0] << " filename" << endl;
        return -1;
    }

    // 通过 stat 函数获取用户传入的文件的信息
    struct stat st;
    int ret = stat(argv[1], &st);
    if(ret == -1) {
        perror("stat");
        return -1;
    }

    // 输出文件大小
    cout << "文件大小: " << st.st_size << " 字节" << endl;

    return 0;
}