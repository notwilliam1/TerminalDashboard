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
        return vbox({
            text(" SYSTEM MONITOR ") | bold | center | border,
            hbox({
                window(text(" CPU Percentage "), text(std::to_string((int)cpuLoad) + "%") | center),
                window(text(""), gaugeRight(cpuLoad/100.0f)),
            }),
            hbox({
                window(text(" Memory Usage "), text(std::format("{:.2f} / {:.2f} GB", GetMemoryUsageGB(), GetTotalMemoryGB())) | center),
                window(text(""), gaugeRight(memoryUsage/memoryTotal)),
            }),
            window(text(" Log "), text(" All systems nominal... "))
        }) | color(Color::GreenLight);
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
