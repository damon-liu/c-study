
 
// int main()
// {
//     std::cout << hello() << std::endl;
// }

#include <vector>
#include <iostream>
#include "hello.hpp"
using namespace std;
 
int main() {
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