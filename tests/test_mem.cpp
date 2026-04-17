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