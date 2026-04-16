#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <filesystem>
#include <windows.h>
#include <thread>
#include <chrono>
#include "system_stats.h"

using namespace ftxui;

int main() {

    auto screen = ScreenInteractive::TerminalOutput();

    auto renderer = Renderer([&] {
        float cpuload = GetCPULoad() * 100.0f;
        return vbox({
            text(" SYSTEM MONITOR ") | bold | center | border,
            hbox({
                window(text(" CPU Percentage "), text(std::to_string((int)cpuload) + "%")),
                window(text(""), gaugeRight(cpuload/100.0f)),
            }),
            window(text(" Log "), text(" All systems nominal... "))
        }) | color(Color::GreenLight);
    });

    std::thread refresh([&] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            screen.PostEvent(Event::Custom);
        }
    });
    refresh.detach();

    screen.Loop(renderer);
    return 0;
}
