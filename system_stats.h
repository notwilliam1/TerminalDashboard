#pragma once
#include <windows.h>
#include <string>
#include <mutex>

float GetCPULoad();
float GetMemoryUsageGB();
float GetTotalMemoryGB();

struct WeatherData {
    double tempF = 0.0;
    std::string condition = "Loading...";
    std::string icon = "🌤️";
    bool loaded = false;
};

struct LocationData {
    double lat = 33.749;
    double lon = -84.388;
    std::string city = "Atlanta";
    bool loaded = false;
};

extern std::mutex weatherMutex;

WeatherData FetchWeather(double lat = 33.749, double lon = -84.388);
LocationData FetchLocation();

struct TimeSnapshot {
    std::string est, pst, gmt, eet, utc8;
};

TimeSnapshot GetCurrentTimes();

float GetDiskUsageGB(const std::string& path = "C:\\");
float GetTotalDiskGB(const std::string& path = "C:\\");
int GetProcessCount();

std::string GetOSVersion();