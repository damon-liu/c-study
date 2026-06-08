# ============================================
# Linux x86_64 交叉编译脚本 (基于 Zig)
# 用法: .\scripts\build_linux.ps1 [-ZigPath <path>] [-Release]
# 示例: .\scripts\build_linux.ps1 -ZigPath D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe -Release
# ============================================

param(
    [string]$ZigPath = "",
    [switch]$Release
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ToolchainFile = "$ProjectRoot/cmake/toolchains/linux-x64.cmake"
$BuildDir = "$ProjectRoot/build_linux"
$BuildType = if ($Release) { "Release" } else { "Debug" }

Write-Host "=== Linux x86_64 交叉编译 ===" -ForegroundColor Cyan
Write-Host "工具链: $ToolchainFile"
Write-Host "构建类型: $BuildType"

# 清理旧构建
if (Test-Path $BuildDir) {
    Write-Host "清理旧构建目录..."
    Remove-Item -Recurse -Force $BuildDir
}

# 构建 CMake 参数
$CmakeArgs = @(
    "-B", $BuildDir,
    "-G", "MinGW Makefiles",
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-S", $ProjectRoot
)

# Zig 路径
if ($ZigPath) {
    $CmakeArgs += "-DZIG_PATH=$ZigPath"
}

# 配置
cmake @CmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake 配置失败!" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 编译
cmake --build $BuildDir

if ($LASTEXITCODE -eq 0) {
    Write-Host "编译成功! 产物: $BuildDir/main" -ForegroundColor Green
    Write-Host "验证: file $BuildDir/main" -ForegroundColor DarkGray
}
