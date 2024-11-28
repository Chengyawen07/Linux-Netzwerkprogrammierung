#include <gtest/gtest.h>
#include "add.h"

// 测试正数相加
TEST(AddTest, PositiveNumbers) {
    EXPECT_EQ(add(1, 2), 3);
}

// 测试负数相加
TEST(AddTest, NegativeNumbers) {
    EXPECT_EQ(add(-1, -2), -3);
}

// 测试零相加
TEST(AddTest, ZeroNumbers) {
    EXPECT_EQ(add(0, 0), 0);
}
