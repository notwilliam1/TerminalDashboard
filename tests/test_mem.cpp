#include <gtest/gtest.h>
#include "system_stats.h"

TEST(GetMemoryUsageGB, ReturnsNonNegative) {
    float result = GetMemoryUsageGB();
    EXPECT_GE(result, 0);
}

TEST(GetTotalMemoryGB, ReturnsNonNegative) {
    float result = GetTotalMemoryGB();
    EXPECT_GE(result, 0);
}

TEST(GetMemoryUsageGB, UsedDoesNotExceedTotal) {
    EXPECT_LE(GetMemoryUsageGB(), GetTotalMemoryGB());
}

TEST(GetTotalMemoryGB, TotalIsReasonable) {
    float result = GetTotalMemoryGB();
    EXPECT_LE(result, 256.0f); // max consumer memory size generally
}

TEST(GetMemoryUsageGB, UsageIsReasonable) {
    float result = GetMemoryUsageGB();
    EXPECT_LE(result, 256.0f);
}