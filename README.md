# DJI MQTT OSD 监控客户端

## 项目概述

基于 Qt6 + C++17 的 DJI MQTT OSD 监控桌面客户端，通过 MQTT 协议实时订阅 DJI 设备的 OSD 数据并进行可视化监控。

| 方式 | 说明 | 适用场景 |
|------|------|---------|
| **本地编译** | Windows + MinGW-w64 + Qt6，生成 `.exe` | 本地开发调试 |
| **本地运行** | windeployqt 部署 Qt DLL 后直接双击运行 | 日常使用 |

---

## 项目结构

```
c-study/
├── CMakeLists.txt                     # 顶层 CMake 构建配置
├── README.md                          # 项目文档
├── .gitignore                         # Git 忽略规则
│
├── src/                               # 源码
│   ├── main.cpp                       # 主程序入口
│   ├── resources/
│   │   └── config.json                # 默认 MQTT 配置文件
│   ├── core/                          # 核心模块
│   │   ├── ConfigStore.h/.cpp         # 配置存储
│   │   ├── DeviceInfo.h               # 设备信息数据结构
│   │   ├── DeviceManager.h/.cpp       # 设备管理器
│   │   ├── OsdData.h                  # OSD 数据结构
│   │   └── TopicManager.h/.cpp        # 主题管理器
│   ├── mqtt/                          # MQTT 模块
│   │   └── MqttClientManager.h/.cpp   # MQTT 客户端管理
│   └── ui/                            # UI 模块
│       ├── MainWindow.h/.cpp          # 主窗口
│       ├── DeviceTreeWidget.h/.cpp    # 设备树控件
│       ├── OsdPanel.h/.cpp            # OSD 面板
│       ├── RawJsonPanel.h/.cpp        # 原始 JSON 面板
│       ├── PublishPanel.h/.cpp        # 发布面板
│       ├── ConfigDialog.h/.cpp        # 配置对话框
│       └── TopicEditDialog.h/.cpp     # 主题编辑对话框
│
├── build_mingw/                       # Windows 构建产物（gitignore）
│   ├── DjiOsdMonitor.exe              # 编译产物
│   └── config.json                    # 运行时配置文件
│
├── deploy/                            # Docker 部署配置（用于 Linux）
│   ├── Dockerfile
│   └── docker-compose.yml
│
└── docs/                              # 设计文档
```

---

## 一、环境准备（Windows）

### 1.1 安装 Qt6

1. 访问 [Qt 官网](https://www.qt.io/download-qt-installer) 下载 Qt Online Installer
2. 安装时选择以下组件：
   - **Qt 6.x.x** → 勾选 `MSVC` 或 `MinGW` 版本（推荐 MinGW）
   - **Qt 6.x.x** → 展开后勾选 `Qt MQTT` 模块
3. 记下安装路径，例如 `D:\tool\c\Qt`

> 本项目开发环境：Qt 6.11.1 (MinGW 64-bit)

### 1.2 配置系统 PATH

将以下两个目录添加到系统环境变量 `Path` 中：

| 路径 | 用途 |
|------|------|
| `<Qt安装目录>\6.x.x\mingw_64\bin` | Qt6 DLL + windeployqt 等工具 |
| `<Qt安装目录>\Tools\mingwXXXX_64\bin` | MinGW 编译器 + 运行时 DLL |

实际示例（以 Qt 6.11.1 安装在 `D:\tool\c\Qt` 为例）：

| 路径 |
|------|
| `D:\tool\c\Qt\6.11.1\mingw_64\bin` |
| `D:\tool\c\Qt\Tools\mingw1310_64\bin` |

**操作步骤**：

1. **Win + Pause** → 点击「高级系统设置」→「环境变量」
2. 在**系统变量**中找到 `Path`，双击编辑
3. 点击「新建」，依次添加上面两个路径
4. 确定保存，**重新打开终端**即可生效

### 1.3 验证环境

重新打开终端，执行以下命令确认配置正确：

```bash
# 检查 CMake
cmake --version

# 检查 Qt6
windeployqt --version

# 检查编译器
g++ --version
```

---

## 二、编译运行

### 2.1 编译

在项目根目录下执行：

```bash
# 配置（Debug 模式，使用 Ninja 生成器）
cmake -B build_mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build_mingw
```

如需 Release 构建：

```bash
cmake -B build_mingw -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build_mingw
```

### 2.2 部署 Qt DLL（windeployqt）

编译产物 `DjiOsdMonitor.exe` 依赖 Qt 动态库，不能直接双击运行，需要用 `windeployqt` 把依赖 DLL 复制到构建目录：

```bash
cmake --build build_mingw --target deploy
```

或手动执行：

```bash
windeployqt --no-translations build_mingw/DjiOsdMonitor.exe
```

### 2.3 运行

部署完成后，直接双击 `build_mingw\DjiOsdMonitor.exe` 即可启动。

或在终端中运行：

```bash
./build_mingw/DjiOsdMonitor.exe
```

---

## 三、配置文件

运行前需修改 `build_mingw/config.json` 中的 MQTT 连接参数：

```json
{
    "mqtt": {
        "host": "192.168.1.100",
        "port": 8883,
        "username": "admin",
        "password": ""
    },
    "devices": []
}
```

| 字段 | 说明 |
|------|------|
| `mqtt.host` | MQTT Broker 地址 |
| `mqtt.port` | MQTT 端口（默认 8883） |
| `mqtt.username` | 用户名 |
| `mqtt.password` | 密码 |
| `devices` | 设备列表（可通过 UI 添加） |

默认配置文件位于 `src/resources/config.json`，CMake 会自动复制到构建目录。

---

## 四、重新编译

```bash
# 只改了源码 → 直接 build
cmake --build build_mingw

# 改了 CMakeLists.txt → 删掉重建
rm -rf build_mingw
cmake -B build_mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build_mingw
cmake --build build_mingw --target deploy
```

---

## 五、CMakeLists.txt 说明

```cmake
cmake_minimum_required(VERSION 3.10)
project(DjiOsdMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)     # 自动处理 Qt MOC（元对象编译）

# 查找 Qt6 组件
find_package(Qt6 COMPONENTS Core Widgets Mqtt REQUIRED)

# 源文件
set(SOURCES
    src/main.cpp
    src/core/ConfigStore.cpp
    src/core/DeviceManager.cpp
    src/core/TopicManager.cpp
    src/mqtt/MqttClientManager.cpp
    src/ui/MainWindow.cpp
    src/ui/DeviceTreeWidget.cpp
    src/ui/OsdPanel.cpp
    src/ui/RawJsonPanel.cpp
    src/ui/PublishPanel.cpp
    src/ui/ConfigDialog.cpp
    src/ui/TopicEditDialog.cpp
)

add_executable(DjiOsdMonitor ${SOURCES} ${HEADERS})

target_link_libraries(DjiOsdMonitor
    Qt6::Core
    Qt6::Widgets
    Qt6::Mqtt
)

# 自动复制配置文件到构建目录
configure_file(
    ${CMAKE_SOURCE_DIR}/src/resources/config.json
    ${CMAKE_BINARY_DIR}/config.json
    COPYONLY
)

# Windows 部署目标（windeployqt 自动打包）
if(WIN32)
    add_custom_target(deploy
        COMMAND windeployqt --no-translations "$<TARGET_FILE:DjiOsdMonitor>"
        DEPENDS DjiOsdMonitor
        COMMENT "Deploying Qt DLLs for Windows distribution"
    )
endif()
```

---

## 六、Docker 部署（Linux）

如需在 Linux 服务器上部署，参考以下架构：

```
Windows 开发机                      Ubuntu 服务器
     │                                  │
     │  1. 交叉编译 main                 │
     │  2. 复制到 deploy/               │
     │                                  │
     │  3. scp deploy/ → 服务器          │
     │─────────────────────────────────>│
     │                                  │  4. docker build
     │                                  │  5. docker compose up
```

> **注意**：当前项目已改为 Qt6 GUI 应用，Docker 部署方案需要调整（GUI 应用无法直接在容器中运行）。此部分内容待更新。

---

## 七、常见问题

### Q: 双击 exe 提示缺少 Qt6Core.dll？

Qt 应用依赖动态库，需要用 `windeployqt` 部署一次：

```bash
cmake --build build_mingw --target deploy
```

### Q: 'windeployqt' 不是内部或外部命令？

Qt6 的 bin 目录没有添加到系统 PATH。参考「1.2 配置系统 PATH」章节。

### Q: CMake 找不到 Qt6？

确保 CMake 能找到 Qt6 的 CMake 配置文件。如果配置了 PATH 后仍然找不到，手动指定：

```bash
cmake -B build_mingw -G Ninja -DCMAKE_PREFIX_PATH=D:/tool/c/Qt/6.11.1/mingw_64
```

### Q: 中文输出乱码？

CMakeLists.txt 中已配置 UTF-8 编译选项，MinGW 下使用 `-fexec-charset=UTF-8`。

### Q: 如何开发时自动连接 MQTT？

在 `src/main.cpp` 中取消下面这行的注释：

```cpp
// devMgr.connectBroker();
```

---

## 八、技术栈

| 技术 | 版本 |
|------|------|
| C++ | C++17 |
| Qt | 6.11.1 (Core / Widgets / Mqtt) |
| CMake | ≥ 3.10 |
| 编译器 | MinGW-w64 (GCC) |
| 构建工具 | Ninja |
