#include "system_stats.h"
#include <windows.h>
#include <cmath>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <format>
#include <mutex>

static unsigned long long FileTimeToInt64(const FILETIME& ft) {
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks) {
    static unsigned long long previousTotalTicks = 0;
    static unsigned long long previousIdleTicks = 0;

    unsigned long long totalTicksSinceLastTime = totalTicks - previousTotalTicks;
    unsigned long long idleTicksSinceLastTime = idleTicks - previousIdleTicks;

    float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

    previousTotalTicks = totalTicks;
    previousIdleTicks = idleTicks;
    return ret;
}

//call every 500ms ish and * by 100.0 for percentage. If cpu load is jumping up to numbers much higher than the actual cpu load, then increase time between refreshes.
float GetCPULoad() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        unsigned long long idle = FileTimeToInt64(idleTime);
        unsigned long long kernel = FileTimeToInt64(kernelTime);
        unsigned long long user = FileTimeToInt64(userTime);
        return CalculateCPULoad(idle, kernel + user);
    }
    return -1.0f;
}

float GetMemoryUsageGB() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    if (GlobalMemoryStatusEx(&memStatus)) {
        unsigned long long used = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        return (float)used / pow(1024, 3);
    }

    return -1.0f;
}

float GetTotalMemoryGB() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        return (float)memStatus.ullTotalPhys / pow(1024, 3);
    }

    return -1.0f;
}