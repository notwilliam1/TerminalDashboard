#include "system_stats.h"
#include <windows.h>
#include <cmath>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <format>
#include <mutex>
#include <cmath>
#include <psapi.h>

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

std::mutex weatherMutex;

static std::pair<std::string, std::string> DecodeWeatherCode(int code) {
    if (code == 0)                return {"Clear",         "☀️"};
    if (code <= 2)                return {"Partly Cloudy", "🌤️"};
    if (code == 3)                return {"Overcast",      "☁️"};
    if (code >= 51 && code <= 67) return {"Rainy",         "🌧️"};
    if (code >= 71 && code <= 77) return {"Snowy",         "🌨️"};
    if (code >= 80 && code <= 82) return {"Showers",       "🌧️"};
    if (code >= 95 && code <= 99) return {"Thunderstorm",  "⛈️"};
    return {"Cloudy", "☁️"};
}

static size_t WriteCallBack(void* contents, size_t size, size_t nmemb, std::string* out) {
    out->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
};

WeatherData FetchWeather(double lat, double lon) {
    WeatherData result;

    CURL* curl = curl_easy_init();
    if (!curl) {
        result.condition = "curl init failed";
        return result;
    }

    std::string url = std::format(
        "https://api.open-meteo.com/v1/forecast"
        "?latitude={:.3f}&longitude={:.3f}"
        "&current=temperature_2m,weathercode"
        "&temperature_unit=fahrenheit",
        lat, lon
    );

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        result.condition = curl_easy_strerror(res);
        return result;
    }

    try {
        auto j = nlohmann::json::parse(response);
        if (j.contains("current")) {
            auto& cur = j["current"];
            if (cur.contains("temperature_2m") && cur.contains("weathercode")) {
                result.tempF = cur["temperature_2m"].get<double>();
                auto [cond, icon] = DecodeWeatherCode(cur["weathercode"].get<int>());
                result.condition  = cond;
                result.icon       = icon;
                result.loaded     = true;
            }
        }
    } catch (...) {
        result.condition = "Parse error";
    }

    return result;
}

LocationData FetchLocation() {
    LocationData result;

    CURL* curl = curl_easy_init();
    if (!curl) return result;

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return result;  // silently falls back to Atlanta defaults

    try {
        auto j = nlohmann::json::parse(response);
        if (j.contains("lat") && j.contains("lon") && j.contains("city")) {
            result.lat    = j["lat"].get<double>();
            result.lon    = j["lon"].get<double>();
            result.city   = j["city"].get<std::string>();
            result.loaded = true;
        }
    } catch (...) {}

    return result;
}

float GetDiskUsageGB(const std::string& path) {
    ULARGE_INTEGER free, total, totalFree;
    if (GetDiskFreeSpaceExA(path.c_str(), &free, &total, &totalFree)) {
        return (float)(total.QuadPart - free.QuadPart) / pow(1024, 3);
    }
    return -1.0f;
}

float GetTotalDiskGB(const std::string& path) {
    ULARGE_INTEGER free, total, totalFree;
    if (GetDiskFreeSpaceExA(path.c_str(), &free, &total, &totalFree)) {
        return (float)total.QuadPart / pow(1024, 3);
    }
    return -1.0f;
}

int GetProcessCount() {
    DWORD processes[1024], cbNeeded;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        return cbNeeded / sizeof(DWORD);
    }
    return 0;
}

std::string GetOSVersion() {
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    auto RtlGetVersion = (LONG(WINAPI*)(PRTL_OSVERSIONINFOW))
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

    if (RtlGetVersion && RtlGetVersion((PRTL_OSVERSIONINFOW)&osvi) == 0) {
        DWORD build = osvi.dwBuildNumber;
        std::string name = (build >= 22000) ? "Windows 11" : "Windows 10";
        return std::format("{} (Build {})", name, build);
    }
    return "Windows (unknown)";
}

