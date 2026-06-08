
 
// int main()
// {
//     std::cout << hello() << std::endl;
// }

#include <vector>
#include <iostream>
#include "hello.hpp"
using namespace std;

#ifdef _WIN32
// 前向声明，避免 #include <windows.h> 与 C++17 std::byte 冲突
extern "C" {
    __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
}
#define CP_UTF8 65001
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // 控制台输出使用 UTF-8（解决中文乱码）
#endif
    cout << hello() << endl;

    int a = 10;
    int b = 20;
    int sum = a + b;
    
    vector<int> nums = {1, 2, 3, 4};
    for (int num : nums) {
        cout << "数字：" << num << endl; // 这里打个断点
    }
 
    cout << "a + b = " << sum << endl;

    return 0;
}