#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <filesystem>
#include <windows.h>
#include <thread>
#include <chrono>
#include "system_stats.h"
#include <format>
#include <curl/curl.h>

using namespace ftxui;

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    auto screen = ScreenInteractive::TerminalOutput();

    std::mutex statsMutex;
    std::atomic<bool> stopFlag = false;

    float cpuLoad = 0.0f;
    float memoryUsage = 0.0f;
    float memoryTotal = 0.0f;
    float memoryGaugeBar = 0.0f;
    std::string OSversion = GetOSVersion();

    LocationData location;
    WeatherData weather;

    std::thread weatherThread([&] {
        location = FetchLocation();
        while (!stopFlag) {
            auto fetched = FetchWeather(
                location.loaded ? location.lat : 33.749,
                location.loaded ? location.lon : -84.388
            );
            {
                std::lock_guard<std::mutex> lock(weatherMutex);
                weather = fetched;
            }
            screen.PostEvent(Event::Custom);
            for (int i = 0; i < 600 && !stopFlag; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    });

    std::thread refresh([&] {
        while (!stopFlag) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            float cpu = GetCPULoad() * 100.0f;
            float memTotal = GetTotalMemoryGB();
            float memUsage = GetMemoryUsageGB();
            {
                std::lock_guard<std::mutex> lock(statsMutex);
                cpuLoad = cpu;
                memoryUsage = memUsage;
                memoryTotal = memTotal;
                memoryGaugeBar = memUsage / memTotal;
            }
            screen.PostEvent(Event::Custom);
        }
    });

    auto renderer = Renderer([&] {
    auto times = GetCurrentTimes();
    float cpuSnap, memUsageSnap, memTotalSnap, gaugeSnap;
    {
        std::lock_guard<std::mutex> lock(statsMutex);
        cpuSnap = cpuLoad;
        memUsageSnap = memoryUsage;
        memTotalSnap = memoryTotal;
        gaugeSnap = memoryGaugeBar;
    }

    auto cellStats = vbox({
        text(" RESOURCE USAGE ") | bold,
        separator(),
        hbox({ text(" CPU  ") | dim | bold, gaugeRight(cpuSnap / 100.0f) | color(Color::GrayDark), text(" " + std::format("{:3d}%", (int)cpuSnap)) }),
        hbox({ text(" RAM  ") | dim | bold, gaugeRight(gaugeSnap) | color(Color::GrayLight), text(" " + std::format("{:.1f} GB", memUsageSnap)) }),
    }) | flex | border;

    WeatherData snap;
    { std::lock_guard<std::mutex> lock(weatherMutex); snap = weather; }

    auto cellWeather = vbox({
        text(" ENVIRONMENT ") | bold,
        separator(),
        snap.loaded ? text(" " + snap.icon + " " + location.city + " " + std::to_string((int)snap.tempF) + "°F") : text(" Syncing..."),
        hbox({ text(" NY ") | dim, text(times.est), text(" CA ") | dim, text(times.pst) }),
        hbox({ text(" LDN ") | dim, text(times.gmt), text(" BJ ") | dim, text(times.utc8) }),
    }) | flex | border;

    float diskUsed = GetDiskUsageGB();
    float diskTotal = GetTotalDiskGB();
    auto cellStorage = vbox({
        text(" STORAGE (C:) ") | bold ,
        separator(),
        hbox({ text(" Used ") | dim | bold, text(std::format("{:.1f} / {:.1f} GB", diskUsed, diskTotal)) }),
        gaugeRight(diskUsed / diskTotal) | color(Color::GrayLight),
    }) | flex | border;

    auto cellSystem = vbox({
        text(" SYSTEM ") | bold,
        separator(),
        text(" Processes: " + std::to_string(GetProcessCount())) | color(Color::White),
        text(" OS: " + OSversion) | dim,
    }) | flex | border;

    return vbox({
        text(" ▼ TERMINAL DASHBOARD v1.0 ") | center | bold | color(Color::Black) | bgcolor(Color::White),
        hbox({ cellStats, cellWeather }),
        hbox({ cellStorage, cellSystem }),
    });
});

    screen.Loop(renderer);
    stopFlag = true;
    weatherThread.join();
    refresh.join();

    curl_global_cleanup();
    return 0;
}
