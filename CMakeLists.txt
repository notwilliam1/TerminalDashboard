cmake_minimum_required(VERSION 3.25)
project(TerminalDashboard)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v6.1.9
)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.17.0
)

FetchContent_Declare(
        curl
        URL https://github.com/curl/curl/releases/download/curl-8_7_1/curl-8.7.1.tar.gz
)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)

FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui curl json googletest)


add_library(system_stats system_stats.cpp
        time_utils.cpp)
target_include_directories(system_stats PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/include)
target_link_libraries(system_stats PUBLIC libcurl nlohmann_json::nlohmann_json)

add_executable(TerminalDashboard main.cpp)
target_link_libraries(TerminalDashboard
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
    PRIVATE system_stats
)

enable_testing()

add_executable(tests tests/test_cpu.cpp
        tests/test_mem.cpp
        tests/test_weather.cpp
)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tests
    PRIVATE gtest
    PRIVATE gtest_main
    PRIVATE system_stats
)
include(GoogleTest)
gtest_discover_tests(tests DISCOVERY_MODE PRE_TEST)