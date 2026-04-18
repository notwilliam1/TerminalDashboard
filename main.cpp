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

    float cpuLoad = 0.0f;
    float memoryUsage = 0.0f;
    float memoryTotal = 0.0f;
    LocationData location;
    WeatherData weather;

    std::thread weatherThread([&] {
        location = FetchLocation();
        while (true) {
            auto fetched = FetchWeather(
                location.loaded ? location.lat : 33.749,
                location.loaded ? location.lon : -84.388
            );
            {
                std::lock_guard<std::mutex> lock(weatherMutex);
                weather = fetched;
            }
            screen.PostEvent(Event::Custom);
            std::this_thread::sleep_for(std::chrono::minutes(10));
        }
    });
    weatherThread.detach();

    std::thread refresh([&] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            cpuLoad = GetCPULoad() * 100.0f;
            memoryTotal = GetTotalMemoryGB();
            memoryUsage = GetMemoryUsageGB();
            screen.PostEvent(Event::Custom);
        }
    });
    refresh.detach();

    auto renderer = Renderer([&] {
        auto cellStats = vbox({
            text(" System Stats ") | bold,
            separator(),
            hbox({ text(" CPU Usage "), text(std::to_string((int)cpuLoad) + "%") | color(Color::GreenLight),
            }), gaugeRight(cpuLoad / 100.0f) | color(Color::GreenLight),
            hbox({ text(" Memory Usage "), text(std::format("{:.2f}/{:.2f} GB", memoryUsage, memoryTotal)) | color(Color::GreenLight),
            }), gaugeRight(memoryUsage / memoryTotal) | color(Color::GreenLight),}) | flex;

        WeatherData snapshot;
        {
            std::lock_guard<std::mutex> lock(weatherMutex);
            snapshot = weather;
        }

        // Time is still placeholder. Use chrono in future to calc times
        auto cellWeather = vbox({
            text(" Weather & Time") | bold,
            separator(),
            snapshot.loaded
                ? text(" " + snapshot.icon + " " + location.city + " " + std::to_string((int)snapshot.tempF) + "°F  " + snapshot.condition) | color(Color::Yellow)
                : text(" Fetching weather...") | dim,
            text(" UTC-5  |  EST 14:32 ") | dim,
            text(" UTC+0  |  GMT 19:32 ") | dim,
            text(" UTC+9  |  JST 04:32 ") | dim,
        }) | flex;

        return vbox({
            text(" SYSTEM MONITOR ") | bold | center,
            separator(),
            hbox({
                cellStats,
                separator(),
                cellWeather,
            }) | flex,
            separator(),
            hbox({}) | flex,
        }) | border | color(Color::GreenLight);
    });

    screen.Loop(renderer);

    curl_global_cleanup();
    return 0;
}
