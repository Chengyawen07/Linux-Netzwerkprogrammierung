#!/bin/bash

# 创建 build 目录
mkdir -p build
cd build

# 运行 CMake 配置和编译
cmake ..
make

# 运行测试
./runTests
