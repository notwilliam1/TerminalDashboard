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

using namespace ftxui;

int main() {

    auto screen = ScreenInteractive::TerminalOutput();

    float cpuLoad = 0.0f;
    float memoryUsage = 0.0f;
    float memoryTotal = 0.0f;

    auto renderer = Renderer([&] {
        auto cellStats = vbox({
            text(" System Stats ") | bold,
            separator(),
            hbox({
            text(" CPU Usage "),
            text(std::to_string((int)cpuLoad) + "%") | color(Color::GreenLight),
            }),
            gaugeRight(cpuLoad / 100.0f) | color(Color::GreenLight),
            hbox({
            text(" Memory Usage "),
            text(std::format("{:.2f}/{:.2f} GB", memoryUsage, memoryTotal)) | color(Color::GreenLight),
            }),
            gaugeRight(memoryUsage / memoryTotal) | color(Color::GreenLight),
        }) | flex;

        return vbox({
            text(" SYSTEM MONITOR ") | bold | center,
            separator(),
            hbox({
            cellStats,
            separator(),
            }) | flex,
            separator(),
            hbox({}) | flex,
        }) | border | color(Color::GreenLight);
    });

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

    screen.Loop(renderer);
    return 0;
}
