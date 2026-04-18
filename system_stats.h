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