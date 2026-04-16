#include <gtest/gtest.h>
#include "system_stats.h"

TEST(CPULoad, ReturnsNonNegative) {
    float result = GetCPULoad() * 100.0f;
    EXPECT_GE(result, 0.0f);
}

TEST(CPULoad, DoesNotExceed100) {
    float result = GetCPULoad() * 100.0f;
    EXPECT_LE(result, 100.0f);
}

TEST(CPULoad, ReturnsValidOnSecondCall) {
    GetCPULoad();
    float result = GetCPULoad() * 100.0f;
    EXPECT_GE(result, 0.0f);
    EXPECT_LE(result, 100.0f);
}