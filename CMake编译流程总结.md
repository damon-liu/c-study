# C++ CMake 项目编译流程总结

## 项目结构

```
c-study/
├── CMakeLists.txt          # CMake 构建配置文件
├── src/
│   ├── main.cpp            # 主程序入口
│   ├── calculator.h        # 计算器类头文件
│   ├── calculator.cpp      # 计算器类实现
│   └── logger.h            # 日志单例类（header-only）
└── build/                  # 构建输出目录（自动生成）
    └── c-study.exe         # 最终可执行文件
```

## 完整编译流程（4 步）

### 第 1 步：编写 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)   # 最低 CMake 版本
project(c-study VERSION 1.0 LANGUAGES CXX)  # 项目名和语言

set(CMAKE_CXX_STANDARD 11)            # 使用 C++11 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # 强制要求该标准

set(SOURCES src/main.cpp src/calculator.cpp)   # 源文件列表
set(HEADERS src/calculator.h src/logger.h)     # 头文件列表

add_executable(c-study ${SOURCES} ${HEADERS})  # 生成可执行文件

find_package(Threads REQUIRED)                 # 查找线程库
target_link_libraries(c-study PRIVATE Threads::Threads)  # 链接线程库
```

### 第 2 步：CMake 配置（生成构建系统）

```bash
mkdir -p build                          # 创建构建目录
cd build
cmake -G "MinGW Makefiles" ..           # 指定生成器，生成 Makefile
```

这一步 CMake 做了：

- 检测编译器（GNU 16.1.0 / g++）
- 检测 C++ 编译特性
- 查找 pthread 线程库
- 生成 `Makefile` 和一系列 `CMakeFiles/` 辅助文件

### 第 3 步：编译链接

```bash
cmake --build .                         # 执行构建
```

实际执行流程：

| 阶段 | 输入 | 输出 |
|------|------|------|
| 预处理 + 编译 | `main.cpp` → | `main.cpp.obj` |
| 预处理 + 编译 | `calculator.cpp` → | `calculator.cpp.obj` |
| 链接 | `main.obj + calculator.obj + libpthread` → | `c-study.exe` |

### 第 4 步：运行

```bash
./c-study.exe
```

> **注意**：在 Windows 上使用 MinGW 编译时，需确保 MinGW 的 `bin/` 目录在 `PATH` 中，否则运行时会因找不到 DLL 报错（exit code 127）。

## 运行结果

```
=== C++ Demo Application ===
Compiled for Linux on: Jun  7 2026 17:54:11
===========================
2026-06-07 17:54:45 [INFO] Application started
2026-06-07 17:54:45 [INFO] Calculator initialized

=== Calculations ===
10 + 5 = 15
10 - 5 = 5
10 * 5 = 50
10 / 5 = 2

=== Solving Quadratic Equation ===
Solutions: 3 2

=== Multi-threading Demo ===
Thread 1 result: 300
Thread 2 result: 20

=== Demo Completed ===
```

## 关键概念

- **CMakeLists.txt** — 声明式的构建描述文件，不直接编译，而是**生成**平台相关的构建文件（Makefile / Visual Studio 解决方案等）
- **Out-of-source build（源外构建）** — 在 `build/` 目录中构建，源码和产物分离，方便清理（删除 `build/` 即可）
- **Generator（生成器）** — `-G "MinGW Makefiles"` 指定生成 MinGW 风格的 Makefile；在 Linux 上通常不需要指定，在 Windows MSVC 环境下会自动生成 Visual Studio 工程
- **find_package + target_link_libraries** — CMake 的依赖管理方式，自动查找系统库并链接

## 常用命令速查

| 命令 | 说明 |
|------|------|
| `cmake -G "MinGW Makefiles" ..` | 配置项目（MinGW） |
| `cmake ..` | 配置项目（默认生成器） |
| `cmake --build .` | 编译项目 |
| `cmake --build . -- -j8` | 8 线程并行编译 |
| `cmake --build . --target clean` | 清理编译产物 |
| `rm -rf build && mkdir build` | 完全重新配置 |
