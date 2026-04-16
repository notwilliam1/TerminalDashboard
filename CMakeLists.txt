cmake_minimum_required(VERSION 4.2)
project(TerminalDashboard)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

add_executable(TerminalDashboard main.cpp
        tests/test_cpu.cpp)

target_link_libraries(TerminalDashboard
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests tests/test_cpu.cpp
        tests/test_cpu.cpp)
target_link_libraries(tests PRIVATE gtest gtest_main)