cmake_minimum_required(VERSION 3.10)
project(c-study VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准为 C++11（项目使用了 std::thread, std::chrono 等）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 收集源文件
set(SOURCES
    src/main.cpp
    src/calculator.cpp
)

set(HEADERS
    src/calculator.h
    src/logger.h
)

# 生成可执行文件
add_executable(c-study ${SOURCES} ${HEADERS})

# 链接线程库（Linux 需要 -pthread）
find_package(Threads REQUIRED)
target_link_libraries(c-study PRIVATE Threads::Threads)
