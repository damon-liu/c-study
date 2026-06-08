# Cross-Compile & Docker Deploy to Ubuntu 24.04 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 c-study C++ 项目交叉编译为 Linux x86_64 可执行文件，并发布到 Ubuntu 24.04 Docker 容器中运行。

**Architecture:** 采用 Docker 多阶段构建（Multi-stage Build）作为"交叉编译"方案。Stage 1 使用 `ubuntu:24.04` + GCC 工具链在容器内编译源码生成 Linux ELF 二进制文件；Stage 2 使用最小化 `ubuntu:24.04` 运行时镜像，仅包含可执行文件及依赖库。编译和运行均在 Docker 内完成，无需在 Windows 宿主机安装 Linux 交叉编译工具链。同时提供 CMake 工具链文件方案用于本地真交叉编译验证。

**Tech Stack:** Docker Desktop (Windows), Ubuntu 24.04, GCC 14, CMake 3.28+, C++11, pthread

---

## 前置检查

- [ ] **Step 0.1: 确认 Docker Desktop 已安装并运行**

```bash
docker version
docker info
```

预期输出: Docker 版本信息，Server 端正常运行。

- [ ] **Step 0.2: 确认当前项目在 Windows 下可正常编译（MinGW）**

```bash
cd d:/project/code/damon/c++/c-study/build
cmake .. -G "MinGW Makefiles"
cmake --build .
./c-study.exe
```

预期输出: 程序正常运行，输出计算和多线程演示结果，生成 `app.log`。

---

## 阶段一：Docker 多阶段构建（推荐交叉编译方案）

### Task 1: 编写 Dockerfile（多阶段构建）

**Files:**
- Create: `Dockerfile`

- [ ] **Step 1.1: 创建 Dockerfile**

在项目根目录创建 `Dockerfile`，采用多阶段构建：第一阶段用完整 Ubuntu 24.04 + 编译工具链编译项目；第二阶段用最小运行时镜像。

```dockerfile
# ============================================================
# Stage 1: Build stage — 交叉编译/构建环境
# ============================================================
FROM ubuntu:24.04 AS builder

# 设置非交互式安装，避免 tzdata 等包卡住
ENV DEBIAN_FRONTEND=noninteractive

# 安装编译工具链
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# 验证工具链版本
RUN g++ --version && cmake --version

WORKDIR /build

# 先复制 CMakeLists.txt 和源码（利用 Docker 层缓存：依赖文件变化才重建）
COPY CMakeLists.txt .
COPY src/ ./src/

# CMake 配置 & 编译（Release 模式，静态链接以减少运行时依赖）
RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=11 \
    && cmake --build build --parallel $(nproc)

# 验证产出是 Linux ELF 二进制
RUN file build/c-study && ldd build/c-study || true

# ============================================================
# Stage 2: Runtime stage — 最小运行时镜像
# ============================================================
FROM ubuntu:24.04 AS runtime

# 安装运行时依赖（libstdc++ 是 C++ 标准库运行时）
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# 创建非 root 用户运行应用
RUN useradd --create-home --shell /bin/bash appuser

WORKDIR /app

# 从构建阶段复制可执行文件
COPY --from=builder /build/build/c-study .

# 创建日志目录并设置权限
RUN mkdir -p /app/logs && chown -R appuser:appuser /app

USER appuser

# 容器启动时运行应用
ENTRYPOINT ["./c-study"]
```

- [ ] **Step 1.2: 创建 .dockerignore**

在项目根目录创建 `.dockerignore`，避免将构建产物和无关文件送入 Docker context：

```dockerignore
build/
.git/
.vscode/
*.exe
*.obj
*.log
CMakeCache.txt
CMakeFiles/
docs/
```

### Task 2: 构建 Docker 镜像（交叉编译验证）

**Files:**
- 验证: `Dockerfile`

- [ ] **Step 2.1: 构建 Stage 1（仅构建阶段，验证交叉编译）**

```bash
cd d:/project/code/damon/c++/c-study
docker build --target builder -t c-study:builder .
```

预期输出: Docker 拉取 ubuntu:24.04 基础镜像，安装 GCC/CMake，编译 C++ 源码成功，`file` 命令输出 `ELF 64-bit LSB executable, x86-64`。

**这是交叉编译验证的关键步骤。** 如果此步成功，说明 Windows 宿主机上的 C++ 源码已被成功编译为 Linux 可执行文件。

- [ ] **Step 2.2: 从 builder 阶段提取并检查二进制文件**

```bash
# 创建临时容器并复制出二进制文件
docker create --name c-study-temp c-study:builder
docker cp c-study-temp:/build/build/c-study ./build/c-study-linux
docker rm c-study-temp

# 检查文件类型
file ./build/c-study-linux
```

预期输出: `./build/c-study-linux: ELF 64-bit LSB executable, x86-64, dynamically linked, ...`

- [ ] **Step 2.3: 构建完整镜像（含运行时阶段）**

```bash
docker build -t c-study:latest .
```

预期输出: 两个阶段均构建成功，最终镜像大小约 80-120 MB。

### Task 3: 运行容器验证（功能测试）

**Files:**
- 验证: 容器运行行为

- [ ] **Step 3.1: 运行容器并验证程序输出**

```bash
docker run --rm c-study:latest
```

预期输出:
```
=== C++ Demo Application ===
Compiled for Linux on: ...
===========================
...
=== Calculations ===
10 + 5 = 15
10 - 5 = 5
10 * 5 = 50
10 / 5 = 2
...
=== Multi-threading Demo ===
Thread 1 result: 300
Thread 2 result: 20
...
=== Demo Completed ===
```

- [ ] **Step 3.2: 验证日志文件写入**

```bash
# 以交互模式运行，挂载本地目录查看日志
docker run --rm -v d:/project/code/damon/c++/c-study/build/logs:/app c-study:latest
# 检查容器内日志
docker run --rm --entrypoint cat c-study:latest /app/app.log
```

预期输出: 日志文件包含 `[INFO]` 时间戳格式的日志条目。

- [ ] **Step 3.3: 验证多线程功能**

确保输出中同时出现 Thread 1 和 Thread 2 的结果（多线程正常工作），运行无明显崩溃或异常。

### Task 4: 落地脚本 — 自动化编译 & 发布

**Files:**
- Create: `scripts/build-and-run.sh` (Linux/Mac)
- Create: `scripts/build-and-run.ps1` (Windows PowerShell)

- [ ] **Step 4.1: 创建 Windows PowerShell 构建脚本**

```powershell
# scripts/build-and-run.ps1
param(
    [string]$Tag = "c-study:latest"
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot/..

Write-Host "=== Step 1: Build Docker Image ===" -ForegroundColor Cyan
docker build -t $Tag .

Write-Host "=== Step 2: Verify Binary Type ===" -ForegroundColor Cyan
docker run --rm --entrypoint file $Tag /app/c-study

Write-Host "=== Step 3: Run Container ===" -ForegroundColor Cyan
docker run --rm $Tag

Write-Host "=== Done ===" -ForegroundColor Green
```

- [ ] **Step 4.2: 创建 Linux 构建脚本**

```bash
#!/bin/bash
# scripts/build-and-run.sh
set -euo pipefail

TAG="${1:-c-study:latest}"
cd "$(dirname "$0")/.."

echo "=== Step 1: Build Docker Image ==="
docker build -t "$TAG" .

echo "=== Step 2: Verify Binary Type ==="
docker run --rm --entrypoint file "$TAG" /app/c-study

echo "=== Step 3: Run Container ==="
docker run --rm "$TAG"

echo "=== Done ==="
```

- [ ] **Step 4.3: 运行构建脚本验证**

```bash
# Windows PowerShell
powershell -ExecutionPolicy Bypass -File scripts/build-and-run.ps1
```

预期: 三步全部通过。

---

## 阶段二：本地真交叉编译（可选，用于 CI/高级场景）

> 如果需要脱离 Docker 在 Windows 上直接生成 Linux 二进制文件，使用以下 CMake 工具链文件方案。

### Task 5: CMake 交叉编译工具链文件（Linux x86_64）

**Files:**
- Create: `cmake/toolchain-linux-x86_64.cmake`

- [ ] **Step 5.1: 创建 CMake 工具链文件**

```cmake
# cmake/toolchain-linux-x86_64.cmake
# 目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# 指定交叉编译器（需预先安装）
# 通过 MSYS2: pacman -S mingw-w64-x86_64-gcc
# 或使用 zig: zig cc --target=x86_64-linux-gnu
set(CMAKE_C_COMPILER    x86_64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER  x86_64-linux-gnu-g++)

# 交叉编译时查找路径
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

- [ ] **Step 5.2: 使用工具链文件编译**

```bash
# 确保已安装 x86_64-linux-gnu-gcc (如通过 MSYS2)
cmake -S . -B build/linux \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
```

- [ ] **Step 5.3: 验证产出文件格式**

```bash
file build/linux/c-study
```

预期输出: `ELF 64-bit LSB executable, x86-64`

---

## 阶段三：发布到 Docker Registry（可选扩展）

### Task 6: 推送到镜像仓库

**Files:**
- 修改: `scripts/build-and-run.ps1`

- [ ] **Step 6.1: 为镜像打标签并推送**

```bash
# 打标签（替换为你的 Docker Hub 用户名或私有仓库地址）
docker tag c-study:latest your-registry/c-study:latest
docker tag c-study:latest your-registry/c-study:$(date +%Y%m%d)

# 推送
docker push your-registry/c-study:latest
docker push your-registry/c-study:$(date +%Y%m%d)
```

- [ ] **Step 6.2: 在目标 Ubuntu 24.04 机器上拉取并运行**

```bash
# 在目标 Linux 服务器上
docker pull your-registry/c-study:latest
docker run --rm your-registry/c-study:latest
```

---

## 附录：验证清单

完成所有 Task 后，逐项确认：

| # | 验证项 | 命令 | 预期 |
|---|--------|------|------|
| 1 | Docker 构建成功 | `docker build -t c-study:latest .` | 无错误 |
| 2 | 产物为 Linux ELF | `docker run --rm --entrypoint file c-study:latest /app/c-study` | `ELF 64-bit LSB executable, x86-64` |
| 3 | 程序正常运行 | `docker run --rm c-study:latest` | 输出计算和线程演示 |
| 4 | 日志写入正常 | `docker run --rm --entrypoint cat c-study:latest /app/app.log` | 含多行 `[INFO]` 日志 |
| 5 | 多线程工作 | 查看 Thread 1/2 输出 | 两个线程结果均出现 |
| 6 | 镜像大小合理 | `docker images c-study:latest` | < 150 MB |
| 7 | 幂等构建 | 第二次 `docker build` 使用缓存 | 1分钟内完成 |
