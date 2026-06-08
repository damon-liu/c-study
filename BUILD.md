# 编译指南

三种终端都使用 `-D` 直接传参，不依赖环境变量，最可靠。

---

## CMD

```cmd
cd /d D:\project\damon\C\Study\c-study

cmake -B build_linux -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=toolchain-linux-x64.cmake -DCMAKE_MAKE_PROGRAM=D:\tool\c\mingw64\bin\mingw32-make.exe -DZIG_PATH=D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe -DCMAKE_BUILD_TYPE=Release

cmake --build build_linux
```

---

## Bash（Git Bash / MSYS2）

```bash
cd /d/project/damon/C/Study/c-study

cmake -B build_linux -G "MinGW Makefiles" \
      -DCMAKE_TOOLCHAIN_FILE=toolchain-linux-x64.cmake \
      -DZIG_PATH=/d/software/c++/lib/zig/zig-windows-x86_64-0.13.0/zig.exe \
      -DCMAKE_BUILD_TYPE=Release

cmake --build build_linux
```

---

## PowerShell

```powershell
cd D:\project\damon\C\Study\c-study

cmake -B build_linux -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=toolchain-linux-x64.cmake -DCMAKE_MAKE_PROGRAM=D:\tool\c\mingw64\bin\mingw32-make.exe -DZIG_PATH=D:\software\c++\lib\zig\zig-windows-x86_64-0.13.0\zig.exe -DCMAKE_BUILD_TYPE=Release

cmake --build build_linux
```

---

## 编译成功后

验证产物：

```bash
file build_linux/main
# 输出: ELF 64-bit LSB executable, x86-64 ...
```

拿到 Ubuntu 24 上运行：

```bash
chmod +x main && ./main
```

## 重新编译

```bash
# 只改了源码 → 直接 build
cmake --build build_linux

# 改了 CMakeLists.txt 或工具链 → 删掉重建
rm -rf build_linux
# 重新执行上面的 cmake -B ... 命令
cmake --build build_linux
```
