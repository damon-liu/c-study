# ============================================
# Windows 本地编译脚本 (MinGW)
# 用法: .\scripts\build_win.ps1
# ============================================

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot

Write-Host "=== Windows 本地编译 ===" -ForegroundColor Cyan
Write-Host "源码目录: $ProjectRoot"

# 默认使用 MinGW Makefiles
$Generator = "MinGW Makefiles"

# 配置
cmake -B "$ProjectRoot/build" -G $Generator `
    -DCMAKE_BUILD_TYPE=Debug `
    -S "$ProjectRoot"

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake 配置失败!" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 编译
cmake --build "$ProjectRoot/build"

if ($LASTEXITCODE -eq 0) {
    Write-Host "编译成功! 产物: $ProjectRoot/build/main.exe" -ForegroundColor Green
}
