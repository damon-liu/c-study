#!/bin/bash
# scripts/build-and-run.sh
# 构建 Docker 镜像并运行验证
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
