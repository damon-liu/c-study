# C++ CMake 项目 → Ubuntu 24 + Docker 发布方案

## 一、现状分析

| 维度 | 当前 | 目标 |
|------|------|------|
| 开发环境 | Windows 11 | — |
| 本地编译器 | MinGW g++ 16.1 | — |
| 构建系统 | CMake 4.3 | CMake |
| C++ 标准 | C++11 | C++11 |
| 依赖 | pthread（系统库） | pthread |
| 目标 OS | — | Ubuntu 24.04 |
| 发布方式 | — | Docker 容器 |

### 现有文件清单

```
c-study/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── calculator.h
│   ├── calculator.cpp
│   └── logger.h
└── build/          # 本地构建产物（不入库）
```

---

## 二、整体方案：多阶段 Docker 构建

```
┌─────────────────────────────────────────────────┐
│  阶段 1: Builder（构建镜像）                      │
│  Ubuntu 24.04 + g++ + cmake + make               │
│  → 编译源码 → 生成 c-study 二进制                 │
└───────────────────┬─────────────────────────────┘
                    │ 复制二进制
┌───────────────────▼─────────────────────────────┐
│  阶段 2: Runtime（运行镜像）                      │
│  Ubuntu 24.04（精简）                             │
│  → 仅包含可执行文件 + 必要运行时库                  │
│  → 镜像体积最小化                                  │
└─────────────────────────────────────────────────┘
```

### 新增文件规划

```
c-study/
├── Dockerfile              # 多阶段构建定义
├── .dockerignore           # 排除不必要文件
├── docker-compose.yml      # 可选：简化运行配置
└── scripts/
    ├── build-docker.sh     # 一键构建脚本（Linux/macOS）
    ├── build-docker.bat    # 一键构建脚本（Windows）
    └── run-docker.sh       # 一键运行脚本
```

---

## 三、文件详细设计

### 3.1 Dockerfile（多阶段构建）

```dockerfile
# ============ 阶段 1：构建阶段 ============
FROM ubuntu:24.04 AS builder

# 安装构建工具链
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 先复制 CMakeLists.txt（利用 Docker 层缓存）
COPY CMakeLists.txt .

# 复制源代码
COPY src/ ./src/

# CMake 配置 + 编译
RUN mkdir -p build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && cmake --build . -- -j$(nproc)

# ============ 阶段 2：运行阶段 ============
FROM ubuntu:24.04 AS runtime

# 安装运行时依赖（仅 pthread 等必要库）
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# 创建非 root 用户（安全最佳实践）
RUN useradd -m -s /bin/bash appuser

WORKDIR /app

# 从构建阶段复制可执行文件
COPY --from=builder /app/build/c-study /app/c-study

# 创建日志目录并设置权限
RUN mkdir -p /app/logs && chown -R appuser:appuser /app

# 切换到非 root 用户
USER appuser

# 容器启动时运行
ENTRYPOINT ["/app/c-study"]
```

### 3.2 .dockerignore

```
# 本地构建产物
build/

# IDE 配置
.vscode/
.idea/

# Git
.git/
.gitignore

# 文档
*.md

# 脚本
scripts/

# 临时文件
*.log
*.tmp
```

### 3.3 docker-compose.yml

```yaml
version: "3.8"

services:
  c-study:
    build:
      context: .
      dockerfile: Dockerfile
    image: c-study:latest
    container_name: c-study-app
    # 挂载日志目录到宿主机
    volumes:
      - ./logs:/app/logs
    # 日志文件写入路径
    environment:
      - LOG_FILE=/app/logs/app.log
    # 重启策略
    restart: unless-stopped
    # 资源限制
    deploy:
      resources:
        limits:
          cpus: "1.0"
          memory: "256M"
```

### 3.4 CMakeLists.txt 调整

为支持 Linux Release 构建，在现有基础上追加：

```cmake
# 追加在现有 CMakeLists.txt 末尾

# Release 构建优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2 -DNDEBUG)
endif()

# 支持通过环境变量设置日志路径
# （在源码中读取 LOG_FILE 环境变量，此处仅做注释说明）
```

### 3.5 可选：源码微调（支持日志路径环境变量）

在 [logger.h](src/logger.h) 的 `setLogFile` 调用前检查环境变量，让 Docker 日志挂载更灵活：

> 此项为可选优化，不影响基本功能。

---

## 四、构建与发布流程

### 4.1 环境准备

| 步骤 | 操作 | 说明 |
|------|------|------|
| 1 | 安装 Docker Desktop | Windows 上需启用 WSL2 后端 |
| 2 | 确认 Docker 可用 | `docker --version` |
| 3 | 确认源码完整 | 4 个源文件 + CMakeLists.txt |

### 4.2 构建 Docker 镜像

```bash
# 在项目根目录执行
docker build -t c-study:latest .

# 构建过程输出：
# [1/2] FROM ubuntu:24.04 AS builder     → 拉取基础镜像
# [2/2] RUN apt-get install ...           → 安装 g++ cmake
# [3/3] COPY ...                          → 复制源码
# [4/4] RUN cmake .. && cmake --build .   → 编译
# [5/5] FROM ubuntu:24.04 AS runtime      → 构建运行镜像
# [6/6] COPY --from=builder ...           → 复制二进制
```

### 4.3 验证镜像

```bash
# 查看镜像大小
docker images c-study

# 预期输出：
# REPOSITORY   TAG       IMAGE ID       CREATED         SIZE
# c-study      latest    abc123def456   2 minutes ago   ~80MB
```

### 4.4 运行容器

```bash
# 基本运行
docker run --rm c-study:latest

# 挂载日志目录运行
docker run --rm -v $(pwd)/logs:/app/logs c-study:latest

# 使用 docker-compose 运行（推荐）
docker-compose up

# 后台运行
docker-compose up -d
```

### 4.5 发布到镜像仓库

```bash
# 标记镜像（以 Docker Hub 为例）
docker tag c-study:latest your-registry/c-study:1.0

# 推送到仓库
docker push your-registry/c-study:1.0

# 在 Ubuntu 24 服务器上拉取并运行
ssh user@ubuntu-server
docker pull your-registry/c-study:1.0
docker run -d -v /var/log/c-study:/app/logs your-registry/c-study:1.0
```

---

## 五、Ubuntu 24 服务器部署

### 5.1 目标服务器准备

```bash
# 在 Ubuntu 24.04 服务器上执行

# 1. 安装 Docker
sudo apt update
sudo apt install -y docker.io docker-compose-v2
sudo systemctl enable --now docker

# 2. 将用户加入 docker 组
sudo usermod -aG docker $USER
newgrp docker

# 3. 创建应用目录
mkdir -p /opt/c-study/logs
```

### 5.2 部署方式选择

| 方式 | 适用场景 | 复杂度 |
|------|----------|--------|
| **A. docker run** | 快速测试、简单部署 | ★☆☆ |
| **B. docker-compose** | 标准化部署、多服务 | ★★☆ |
| **C. systemd + docker** | 生产环境、开机自启 | ★★★ |

#### 方式 A：docker run

```bash
docker run -d \
  --name c-study \
  --restart unless-stopped \
  -v /opt/c-study/logs:/app/logs \
  your-registry/c-study:1.0
```

#### 方式 B：docker-compose（推荐）

```bash
# 将 docker-compose.yml 复制到服务器
scp docker-compose.yml user@server:/opt/c-study/

# 在服务器上启动
ssh user@server
cd /opt/c-study
docker compose up -d
```

#### 方式 C：systemd 服务（生产级）

```ini
# /etc/systemd/system/c-study.service
[Unit]
Description=C++ Study Demo Application
Requires=docker.service
After=docker.service

[Service]
Restart=always
ExecStartPre=-/usr/bin/docker rm -f c-study
ExecStart=/usr/bin/docker run --rm --name c-study \
  -v /opt/c-study/logs:/app/logs \
  your-registry/c-study:1.0
ExecStop=/usr/bin/docker stop c-study

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now c-study
```

---

## 六、完整操作步骤时间线

```
第 1 步  创建 Dockerfile + .dockerignore          ← 一次性
第 2 步  本地构建测试：docker build -t c-study .   ← 每次改代码后
第 3 步  本地运行验证：docker run --rm c-study     ← 每次构建后
第 4 步  打标签 + 推送镜像到仓库                    ← 发布时
第 5 步  在 Ubuntu 24 服务器上拉取镜像并运行        ← 部署时
```

---

## 七、镜像体积优化（可选进阶）

| 手段 | 预期体积 | 复杂度 |
|------|----------|--------|
| 当前方案（ubuntu:24.04 runtime） | ~80 MB | ★☆☆ |
| 改用 `alpine:3.20` 运行时 | ~15 MB | ★★☆ |
| 静态链接 + `scratch` 镜像 | ~5 MB | ★★★ |

### Alpine 运行时方案（替代 runtime 阶段）

```dockerfile
FROM alpine:3.20 AS runtime
RUN apk add --no-cache libstdc++
RUN adduser -D appuser
WORKDIR /app
COPY --from=builder /app/build/c-study /app/c-study
USER appuser
ENTRYPOINT ["/app/c-study"]
```

> ⚠️ Alpine 使用 musl libc 而非 glibc，需要确认依赖兼容性。

---

## 八、常见问题排查

| 问题 | 原因 | 解决 |
|------|------|------|
| `docker: command not found` | 未安装 Docker | 安装 Docker Desktop 或 docker.io |
| `CMake Error: Threads not found` | 构建阶段缺少 libc-dev | Dockerfile 中确保安装了 `build-essential` |
| 容器启动后立即退出 | 程序执行完就结束（非守护进程） | 正常行为；查看日志用 `docker logs c-study` |
| 日志文件找不到 | 未挂载卷 | 使用 `-v` 或 docker-compose volumes |
| `exec format error` | 在 ARM 机器上运行 x86 镜像 | 构建时使用 `--platform linux/amd64` |

---

## 九、附录：一键构建脚本

### build-docker.bat（Windows）

```bat
@echo off
echo === Building C-Study Docker Image ===
docker build -t c-study:latest .
if %ERRORLEVEL% EQU 0 (
    echo === Build Successful ===
    echo === Running Test ===
    docker run --rm c-study:latest
) else (
    echo === Build Failed ===
    pause
)
```

### build-docker.sh（Linux/macOS）

```bash
#!/bin/bash
set -e
echo "=== Building C-Study Docker Image ==="
docker build -t c-study:latest .
echo "=== Build Successful ==="
echo "=== Running Test ==="
docker run --rm c-study:latest
```
