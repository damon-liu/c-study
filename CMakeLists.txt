
# ============================================
# C-Study 项目构建配置
# ============================================

cmake_minimum_required(VERSION 3.10)
project(demo1 VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认 Debug 构建（可通过 -DCMAKE_BUILD_TYPE=Release 覆盖）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ============================================
# 可执行文件
# ============================================
add_executable(main
    src/hello.cpp
    src/main.cpp
)

# 头文件搜索路径
target_include_directories(main PRIVATE src)

# ============================================
# 编译器标志
# ============================================
if(MSVC)
    target_compile_options(main PRIVATE /utf-8)
else()
    target_compile_options(main PRIVATE -fexec-charset=UTF-8)
endif()

# ============================================
# 编译后复制到 deploy/ (用于 Docker 部署)
# ============================================
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:main>"
        "${CMAKE_SOURCE_DIR}/deploy/$<TARGET_FILE_NAME:main>"
    COMMENT "Copying $<TARGET_FILE_NAME:main> to deploy/"
)
