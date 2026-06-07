
# 最低 CMake 版本要求
cmake_minimum_required(VERSION 3.10)
# 项目名称
project(demo1)
# 设置 C++ 标准（与代码匹配）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 生成可执行文件：main（对应源码文件）
add_executable(main hello.cpp main.cpp)

# 关键：开启 Debug 模式，生成调试符号（必须！）
set(CMAKE_BUILD_TYPE Debug)
