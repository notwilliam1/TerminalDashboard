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
            text(" System Stats ") | color(Color::White) | bold,
            separator(),
            hbox({ text(" CPU Usage: ") | color(Color::White) | dim, text(std::to_string((int)cpuSnap) + "%") | color(Color::White),}),
                    gaugeRight(cpuSnap / 100.0f) | color(Color::Green3Bis),
            hbox({ text(" Memory Usage: ") | color(Color::White) | dim, text(std::format("{:.2f}/{:.2f} GB", memUsageSnap, memTotalSnap)) | color(Color::White),}),
                    gaugeRight(gaugeSnap) | color(Color::Green3Bis),}) | flex;

        WeatherData snapshot;
        {
            std::lock_guard<std::mutex> lock(weatherMutex);
            snapshot = weather;
        }

        auto cellWeather = vbox({
            text(" Weather & Time ") | bold,
            separator(),
            snapshot.loaded
                ? text(" " + snapshot.icon + " " + location.city + " " + std::to_string((int)snapshot.tempF) + "°F  " + snapshot.condition) | color(Color::Yellow) | bold
                : text(" Fetching weather... ") | dim,
            hbox({ text(" New York ") | color(Color::White) | dim, text(times.est) | color(Color::White) | bold,
                    text(" | California ") | color(Color::White) | dim, text(times.pst) | color(Color::White) | bold }),
            hbox({ text(" United Kingdom ") | color(Color::White) | dim, text(times.gmt) | color(Color::White) | bold,
                    text(" | Greece ") | color(Color::White) | dim, text(times.eet) | color(Color::White) | bold }),
            hbox({ text(" Beijing ") | color(Color::White) | dim, text(times.utc8) | color(Color::White) | bold }),
        }) | flex;

        return vbox({
            text(" SYSTEM MONITOR ") | bold | center | color(Color::White),
            separator(),
            hbox({
                cellStats,
                separator(),
                cellWeather,
            }) | flex,
            separator(),
            hbox({}) | flex,
        }) | border | color(Color::White);
    });

    screen.Loop(renderer);
    stopFlag = true;
    weatherThread.join();
    refresh.join();

    curl_global_cleanup();
    return 0;
}
