cmake_minimum_required(VERSION 3.0)
project(mymuduo)

# 设置动态库路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息 以及 启动c++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 定义参与编译的源代码文件
aux_source_directory(. SRC_LIST)

include_directories(${PROJECT_SOURCE_DIR})

# 编译生成动态库
add_library(mymuduo SHARED ${SRC_LIST})
