# ============================================
# Linux x86_64 交叉编译工具链文件 (基于 Zig)
#
# 用法:
#   cmake -B build_linux -G "MinGW Makefiles" \
#         -DCMAKE_TOOLCHAIN_FILE=toolchain-linux-x64.cmake \
#         -DZIG_PATH=/path/to/zig.exe
#   cmake --build build_linux
# ============================================

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_SYSTEM_VERSION 1)

# Zig 路径（优先级: -DZIG_PATH= > 环境变量 > PATH 搜索）
if(NOT ZIG_PATH AND DEFINED ENV{ZIG_PATH})
    set(ZIG_PATH "$ENV{ZIG_PATH}" CACHE FILEPATH "zig.exe 的路径")
endif()
if(NOT ZIG_PATH)
    find_program(ZIG_PATH zig)
endif()

if(NOT ZIG_PATH)
    message(FATAL_ERROR
        "未找到 zig！\n"
        "  下载: https://ziglang.org/download/\n"
        "  或: set ZIG_PATH=D:\\path\\to\\zig.exe")
endif()

message(STATUS "Zig 编译器: ${ZIG_PATH}")

# 目标平台（Ubuntu 24 = glibc 2.39）
set(TARGET_TRIPLE "x86_64-linux-gnu.2.39")

# === 编译器 ===
set(CMAKE_C_COMPILER "${ZIG_PATH}")
set(CMAKE_CXX_COMPILER "${ZIG_PATH}")

# zig cc / zig c++ — 子命令作为 ARG1
# CMake 在编译和链接时都会在 <COMPILER> 之后插入 ARG1
set(CMAKE_C_COMPILER_ARG1 "cc")
set(CMAKE_CXX_COMPILER_ARG1 "c++")

# === 编译与链接标志 ===
# CMAKE_C_FLAGS/_INIT → 编译阶段
# CMAKE_EXE_LINKER_FLAGS/_INIT → 链接阶段
# 两者的 -target 都要设置（编译和链接是分开的）
set(CMAKE_C_FLAGS_INIT "-target ${TARGET_TRIPLE}")
set(CMAKE_CXX_FLAGS_INIT "-target ${TARGET_TRIPLE}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-target ${TARGET_TRIPLE}")

# 静态链接 libc++ / libunwind（避免目标系统缺少依赖）
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -static")

# === 交叉编译 ===
set(CMAKE_CROSSCOMPILING TRUE)
# 关键：交叉编译时 try_compile 只编译不链接
# 否则 try_compile 子项目重新加载工具链会丢失 -D 变量，导致找不到编译器
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# === C++ 标准 ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
