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

# CMake 配置 & 编译（Release 模式）
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
