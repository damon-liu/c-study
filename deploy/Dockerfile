# ============================================
# Dockerfile - C++ demo 应用容器化部署
# 基础镜像: Ubuntu 24.04 (glibc 2.39)
# ============================================

FROM ubuntu:24.04

# 安装运行时依赖（动态链接的 C++ 程序需要 libstdc++）
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libstdc++6 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 创建应用目录
WORKDIR /app

# 拷贝编译好的二进制文件
COPY main .

# 确保可执行
RUN chmod +x main

# 容器启动时执行
ENTRYPOINT ["./main"]
