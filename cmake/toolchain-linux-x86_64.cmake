# cmake/toolchain-linux-x86_64.cmake
# CMake 交叉编译工具链文件 — Windows 宿主机 → Linux x86_64 目标
#
# 前置条件：安装 Linux 交叉编译器
#   MSYS2:  pacman -S mingw-w64-x86_64-gcc
#   或使用: zig cc --target=x86_64-linux-gnu
#
# 用法:
#   cmake -S . -B build/linux \
#       -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-x86_64.cmake \
#       -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/linux

# 目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# 指定交叉编译器（需预先安装）
set(CMAKE_C_COMPILER    x86_64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER  x86_64-linux-gnu-g++)

# 交叉编译时查找路径策略
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
