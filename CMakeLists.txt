
# 最低 CMake 版本要求
cmake_minimum_required(VERSION 3.10)
# 项目名称
project(demo1)
# 设置 C++ 标准（与代码匹配）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 生成可执行文件：main（对应源码文件）
add_executable(main hello.cpp main.cpp)

# 强制使用 UTF-8 编码（解决中文乱码）
if(MSVC)
    target_compile_options(main PRIVATE /utf-8)
else()
    # MinGW/GCC: 确保执行字符集为 UTF-8
    target_compile_options(main PRIVATE -fexec-charset=UTF-8)
endif()

# 关键：开启 Debug 模式，生成调试符号（必须！）
set(CMAKE_BUILD_TYPE Debug)
