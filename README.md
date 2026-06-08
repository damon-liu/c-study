# C-Study 交叉编译指南

## 项目概述

本项目是一个 C++ 学习项目，使用 CMake 构建，支持**本地编译**（Windows）和**交叉编译**（Linux）。

## 项目结构

```
c-study/
├── CMakeLists.txt                     # 顶层 CMake 构建配置
├── README.md                          # 项目文档
├── .gitignore                         # Git 忽略规则
│
├── cmake/                             # CMake 模块 & 工具链
│   └── toolchains/
│       └── linux-x64.cmake            # Linux x86_64 交叉编译工具链（基于 Zig）
│
├── src/                               # 源码
│   ├── main.cpp                       # 主程序入口
│   ├── hello.cpp                      # hello 模块实现
│   └── hello.hpp                      # hello 模块头文件
│
├── scripts/                           # 构建 & CI 脚本
│   ├── build_win.ps1                  # Windows 本地一键构建
│   └── build_linux.ps1                # Linux 交叉编译一键构建
│
├── deploy/                            # Docker 部署配置
│   ├── Dockerfile
│   └── docker-compose.yml
│
├── tests/                             # 单元测试（预留）
├── docs/                              # 补充文档（预留）
│
├── build/                             # Windows 本地构建产物（gitignore）
│   └── main.exe
└── build_linux/                       # Linux 交叉编译产物（gitignore）
    └── main
```

---

## 一、本地编译（Windows）

### 前置条件

- MinGW-w64（g++）
- CMake ≥ 3.10

### 一键编译

```powershell
.\scripts\build_win.ps1
```

### 手动编译

```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### 运行

```bash
./build/main.exe
```

输出：

```
Hello, world!
数字：1
数字：2
数字：3
数字：4
a + b = 30
```

---

## 二、交叉编译（Windows → Linux）

### 原理

```
┌─────────────────────────────────────────────────────┐
│                     Windows 主机                      │
│                                                       │
│   src/main.cpp ──────────────┐                        │
│   src/hello.cpp ─────────────┤                        │
│   src/hello.hpp ─────────────┤                        │
│                               ▼                       │
│   CMakeLists.txt  +  cmake/toolchains/linux-x64.cmake │
│       │                         │                     │
│       │                         ├─ CMAKE_SYSTEM_NAME  │
│       │                         │   = Linux           │
│       │                         ├─ CMAKE_CXX_COMPILER │
│       │                         │   = zig c++         │
│       │                         └─ -target            │
│       │                             x86_64-linux-gnu  │
│       ▼                                               │
│   zig c++（内置 clang + lld + libc）                  │
│       │                                               │
│       │  源码按 Linux ABI 编译                        │
│       │  链接 Linux libc.so.6                         │
│       ▼                                               │
│   main（ELF 64-bit）──────►  拿到任何 Linux x86_64    │
│                                直接运行               │
└─────────────────────────────────────────────────────┘
```

### 为什么用 Zig

| 方案 | 复杂度 | 说明 |
|------|--------|------|
| 传统交叉编译器 | 高 | 需要目标系统的头文件和库（sysroot），不同 glibc 版本还要区分 |
| WSL 原生编译 | 低 | 需要安装完整的 Linux 发行版（几 GB） |
| **Zig 交叉编译** | 低 | 一个 ~70MB 的 `zig.exe`，自带 clang + lld + 所有目标 libc，零配置 |

Zig 内置了 clang 编译器和 lld 链接器，并且打包了所有目标平台的 libc 头文件和库。**一个二进制文件搞定交叉编译，不需要安装任何 Linux 的库或头文件。**

### 前置条件

1. MinGW-w64（用于 `mingw32-make`）
2. CMake ≥ 3.10
3. [Zig](https://ziglang.org/download/)（下载 `zig-windows-x86_64-*.zip`，解压到任意目录）

### 一键编译

```powershell
# Debug 构建
.\scripts\build_linux.ps1 -ZigPath D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe

# Release 构建
.\scripts\build_linux.ps1 -ZigPath D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe -Release
```

### 手动编译

**Bash（Git Bash / MSYS2）：**

```bash
cmake -B build_linux -G "MinGW Makefiles" \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake \
      -DZIG_PATH=/d/software/c++/lib/zig/zig-windows-x86_64-0.13.0/zig.exe \
      -DCMAKE_BUILD_TYPE=Release

cmake --build build_linux
```

**CMD：**

```cmd
cmake -B build_linux -G "MinGW Makefiles" ^
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake ^
      -DCMAKE_MAKE_PROGRAM=D:\tool\c\mingw64\bin\mingw32-make.exe ^
      -DZIG_PATH=D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe ^
      -DCMAKE_BUILD_TYPE=Release

cmake --build build_linux
```

**PowerShell：**

```powershell
cmake -B build_linux -G "MinGW Makefiles" `
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake `
      -DCMAKE_MAKE_PROGRAM=D:\tool\c\mingw64\bin\mingw32-make.exe `
      -DZIG_PATH=D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe `
      -DCMAKE_BUILD_TYPE=Release

cmake --build build_linux
```

> **提示：** 如果已将 `ZIG_PATH` 设为环境变量（`export ZIG_PATH=...`），可省略 `-DZIG_PATH=...` 参数，工具链文件会自动读取。

产物：`build_linux/main`（ELF 64-bit LSB executable, x86-64）

### 验证产物

```bash
# 确认是 Linux ELF 格式
file build_linux/main
# 输出: ELF 64-bit LSB executable, x86-64, dynamically linked ...

# 查看动态库依赖
objdump -p build_linux/main | grep NEEDED
# NEEDED  libpthread.so.0
# NEEDED  libc.so.6
```

依赖只有 `libc.so.6` 和 `libpthread.so.0`，这是任何 Linux 系统的标准库，不依赖外部第三方库。

### 部署到 Linux

```bash
# 复制到目标 Linux 机器
scp build_linux/main user@ubuntu-server:/home/user/

# 在 Linux 上运行
ssh user@ubuntu-server
chmod +x main && ./main
```

### 重新编译

```bash
# 只改了源码 → 直接 build
cmake --build build_linux

# 改了 CMakeLists.txt 或工具链 → 删掉重建
rm -rf build_linux
cmake -B build_linux -G "MinGW Makefiles" \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake \
      -DZIG_PATH=/d/software/c++/lib/zig/zig-windows-x86_64-0.13.0/zig.exe
cmake --build build_linux
```

---

## 三、CMakeLists.txt 说明

```cmake
cmake_minimum_required(VERSION 3.10)
project(demo1 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认 Debug，可通过 -DCMAKE_BUILD_TYPE=Release 覆盖
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(main
    src/hello.cpp
    src/main.cpp
)

target_include_directories(main PRIVATE src)

# MSVC: 强制 UTF-8 编码（解决中文乱码）
if(MSVC)
    target_compile_options(main PRIVATE /utf-8)
else()
    target_compile_options(main PRIVATE -fexec-charset=UTF-8)
endif()

# 编译后自动复制到 deploy/ 目录（用于 Docker 部署）
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:main>"
        "${CMAKE_SOURCE_DIR}/deploy/$<TARGET_FILE_NAME:main>"
    COMMENT "Copying $<TARGET_FILE_NAME:main> to deploy/"
)
```

**UTF-8 处理要点**：

| 编译器 | 标志 | 作用 |
|--------|------|------|
| MSVC | `/utf-8` | 同时设置源文件字符集和执行字符集为 UTF-8 |
| GCC/MinGW | `-fexec-charset=UTF-8` | 字符串字面量按 UTF-8 编码嵌入二进制 |

`src/main.cpp` 中的运行时设置：

```cpp
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
}
#define CP_UTF8 65001
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // 控制台输出使用 UTF-8
#endif
    // ...
}
```

> **为什么需要两层修复？**
>
> | 层级 | 解决什么 | 不加会怎样 |
> |------|---------|-----------|
> | 编译时 `-fexec-charset=UTF-8` | 字符串 "数字：" 在二进制中存为 UTF-8 | 编译器可能按 GBK 编码，源头就乱了 |
> | 运行时 `SetConsoleOutputCP(65001)` | 控制台按 UTF-8 解码显示 | 控制台默认用 GBK (code page 936) 解码 UTF-8 字节，显示乱码 |

---

## 四、交叉编译工具链说明

`cmake/toolchains/linux-x64.cmake`：

```cmake
# 目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Zig 路径查找（优先级: -DZIG_PATH > 环境变量 > PATH）
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ZIG_PATH)
if(NOT ZIG_PATH)
    if(DEFINED ENV{ZIG_PATH})
        set(ZIG_PATH "$ENV{ZIG_PATH}" CACHE FILEPATH "Zig 编译器路径")
    else()
        find_program(ZIG_PATH zig)
    endif()
endif()

# 编译器：zig cc / zig c++
set(CMAKE_C_COMPILER   "${ZIG_PATH}")
set(CMAKE_CXX_COMPILER "${ZIG_PATH}")
set(CMAKE_C_COMPILER_ARG1   "cc")
set(CMAKE_CXX_COMPILER_ARG1 "c++")
set(CMAKE_ASM_COMPILER "${ZIG_PATH}")

# 目标三元组
set(TARGET_TRIPLE "x86_64-linux-gnu.2.39")
set(CMAKE_C_FLAGS_INIT   "-target ${TARGET_TRIPLE}")
set(CMAKE_CXX_FLAGS_INIT "-target ${TARGET_TRIPLE}")

# 交叉编译标记
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

### Target Triple 说明

Zig 使用 LLVM target triple 格式：

```
x86_64-linux-gnu.2.39
  │      │     │   │
  │      │     │   └─ glibc 版本（2.39 = Ubuntu 24）
  │      │     └─ ABI（gnu = glibc, musl = musl libc）
  │      └─ 操作系统
  └─ CPU 架构
```

常见变体：

| Triple | 适用系统 |
|--------|---------|
| `x86_64-linux-gnu.2.39` | Ubuntu 24, Debian 13 |
| `x86_64-linux-gnu.2.31` | Ubuntu 20, CentOS 8 |
| `x86_64-linux-musl` | Alpine Linux，或静态编译（零依赖） |
| `aarch64-linux-gnu` | ARM64 Linux（树莓派 4/5） |

---

## 五、常见问题

### Q: 中文输出乱码？

编译时添加 `-fexec-charset=UTF-8`（MinGW）或 `/utf-8`（MSVC），同时运行时调用 `SetConsoleOutputCP(CP_UTF8)`。

### Q: `std::byte` 编译错误？

MinGW 下 `#include <windows.h>` 与 C++17 `std::byte` 冲突。使用前向声明代替：

```cpp
extern "C" {
    __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
}
```

### Q: Zig 找不到？

```bash
# 方式1：环境变量（推荐）
export ZIG_PATH=/d/software/c++/lib/zig/zig-windows-x86_64-0.13.0/zig.exe

# 方式2：CMake 直接传参
cmake -B build_linux ... -DZIG_PATH=/d/software/c++/lib/zig/zig-windows-x86_64-0.13.0/zig.exe
```

### Q: 想编译成完全静态的 Linux 二进制？

在 `cmake/toolchains/linux-x64.cmake` 中将目标三元组改为 musl：

```cmake
set(TARGET_TRIPLE "x86_64-linux-musl")
```

musl 是静态友好的 libc 实现，编译出的二进制零动态依赖，可在任何 Linux x86_64 系统上直接运行。

### Q: CMake 报 "ABI detection" 或 try_compile 错误？

通常是 Zig 路径在 try_compile 子构建中丢失。当前工具链已通过 `CMAKE_TRY_COMPILE_PLATFORM_VARIABLES` 解决——确保使用最新版 `cmake/toolchains/linux-x64.cmake`。
