cmake_minimum_required(VERSION 3.10)
project(AddTestProject)

# 添加 Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加源文件和测试文件
add_executable(runTests add.cpp test_add.cpp)

# 链接 GTest 库和 GTest Main 库
target_link_libraries(runTests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARY} pthread)

