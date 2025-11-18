#include <gtest/gtest.h>
#include "utils.hpp"
#include <vector>
#include <cstring>

TEST(UtilsTest, CapValueMacro) {
    EXPECT_EQ(cap_value(5, 1, 10), 5);
    EXPECT_EQ(cap_value(-5, 0, 10), 0);
    EXPECT_EQ(cap_value(15, 0, 10), 10);
}

TEST(UtilsTest, ApplyRateMacros) {
    EXPECT_EQ(apply_rate(100, 100), 100);
    EXPECT_EQ(apply_rate(200, 50), 100);
    EXPECT_EQ(apply_rate2(200, 50, 200), 50);
    EXPECT_EQ(apply_rate2(1000000, 50, 100), 500000); // Large value branch
}

TEST(UtilsTest, GetPercentage) {
    EXPECT_EQ(get_percentage(50, 100), 50);
    EXPECT_EQ(get_percentage(0, 100), 0);
    EXPECT_EQ(get_percentage(100, 0), 0); // Division by zero handled as 0
}

TEST(UtilsTest, GetPercentageExp) {
    EXPECT_EQ(get_percentage_exp(100ULL, 200ULL), 50);
    EXPECT_EQ(get_percentage_exp(0ULL, 100ULL), 0);
    EXPECT_EQ(get_percentage_exp(100ULL, 0ULL), 0);
}

TEST(UtilsTest, ByteWordDwordAccess) {
    uint32 val = 0x12345678;
    EXPECT_EQ(GetByte(val, 0), 0x78);
    EXPECT_EQ(GetByte(val, 1), 0x56);
    EXPECT_EQ(GetWord(val, 0), 0x5678);
    EXPECT_EQ(GetWord(val, 1), 0x1234);
    EXPECT_EQ(MakeWord(0x78, 0x56), 0x5678);
    EXPECT_EQ(MakeDWord(0x5678, 0x1234), 0x12345678);
}

TEST(UtilsTest, ExistsCheck) {
    // Should exist: this test file itself
    EXPECT_TRUE(exists("tests/test_utils.cpp"));
    // Should not exist
    EXPECT_FALSE(exists("tests/this_file_does_not_exist.txt"));
}