# Terminal Dashboard

A terminal dashboard built in C++23 that displays live system metrics, weather, and world clocks.

![Platform](https://img.shields.io/badge/platform-Windows-blue) ![C++](https://img.shields.io/badge/C%2B%2B-23-blue) ![License](https://img.shields.io/badge/license-MIT-green) ![Build](https://img.shields.io/github/actions/workflow/status/notwilliam1/TerminalDashboard/release.yaml)

---

![Terminal Dashboard Screenshot](images/Screenshot%202026-04-23%20033005.png)

---

## Why I built this
This project started as a way to combine system monitoring with multi-threaded API fetching. 
I needed a way to keep an eye on my disk space (which is always low) and CPU spikes during 
builds, but I also wanted to integrate a world clock for the friends I talk to in different 
timezones and keep an eye on the weather. It’s become my go-to utility to leave running on my second monitor while I'm 
coding or doing schoolwork.

## Features

- **Live CPU monitoring** — polls `GetSystemTimes` to compute real utilization with a delta-based algorithm, updating every second
- **Memory usage** — reports used and total physical RAM in GB using `GlobalMemoryStatusEx`
- **Disk usage** — displays used vs. total space on `C:\` with a gauge bar
- **Real-time weather** — fetches current temperature and conditions from [Open-Meteo](https://open-meteo.com/) (no API key required), with automatic IP-based location detection via [ip-api.com](http://ip-api.com/)
- **World clocks** — shows the current time across EST, PST, GMT, EET, and UTC+8
- **Process count** — live count of running system processes via `EnumProcesses`
- **OS version detection** — reads build number directly from `ntdll.dll` via `RtlGetVersion` to correctly identify Windows 10 vs. Windows 11
- **Graceful fallback** — defaults to Atlanta coordinates if location fetch fails; all network operations time out cleanly

## Tech Stack

| Component | Technology |
|---|---|
| TUI rendering | [FTXUI](https://github.com/ArthurSonzogni/FTXUI) v6.1.9 |
| HTTP client | [libcurl](https://curl.se/) 8.7.1 (Schannel, static) |
| JSON parsing | [nlohmann/json](https://github.com/nlohmann/json) 3.12.0 |
| Testing | [GoogleTest](https://github.com/google/googletest) 1.17.0 |
| Build system | CMake 3.25+ |
| Standard | C++23 |
| CI/CD | GitHub Actions |

All dependencies are fetched and built automatically via CMake's `FetchContent` so no manual installs required.

## Architecture

```
TerminalDashboard/
├── main.cpp              # UI layout, render loop, background threads
├── system_stats.h        # Public API surface
├── system_stats.cpp      # CPU, memory, disk, weather, location, OS
├── time_utils.cpp        # Multi-timezone clock snapshot
├── CMakeLists.txt        # Build + dependency management
├── .github/
│   └── workflows/
│       └── release.yaml  # Tag-triggered CI/CD build and release
└── tests/
    ├── test_cpu.cpp
    ├── test_mem.cpp
    └── test_weather.cpp
```

The application runs two background threads alongside the main render loop:

- **Refresh thread** — samples CPU and memory every second, writes to shared state under a mutex, then posts a render event
- **Weather thread** — fetches location once on startup, then re-fetches weather every 10 minutes without blocking the UI

## Limitations
- Currently only supports the C: drive for disk usage
- Only supports Windows 10 and 11. Does not support Linux or macOS
- When scaling the size of the terminal, the ui can temporarily distort

## Future Improvements
- Add support for Linux and macOS
- Add support for more disk drives
- Give options to choose which timezones to display

## Getting Started

### Note
- Since the binary is unsigned, Windows may show a "SmartScreen" warning. You can click 'More Info' -> 'Run Anyway' to start the dashboard.

### Download (Quick Start)
- Download Latest Release: [TerminalDashboard.exe](https://github.com/notwilliam1/TerminalDashboard/releases/latest)
- Run!

### Prerequisites for building from source

- Windows 10/11
- [CMake 3.25+](https://cmake.org/download/)
- [Visual Studio 2022](https://visualstudio.microsoft.com/) (MSVC)

### Via CLion (I Recommend)
- Open the project in CLion
- CLion will pull dependencies automatically
- Run!

### Via Command Line

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Run Tests

```bash
cd build
ctest -C Release --output-on-failure
```

Tests cover CPU load bounds, memory validity, weather struct initialization, and network fetch behavior (network tests auto-skip if offline).

## CI/CD

Pushing a tag matching `v*` triggers a GitHub Actions workflow that builds a Release binary on `windows-latest` and attaches `TerminalDashboard.exe` directly to the GitHub Release.

```bash
git tag v1.0
git push origin v1.0
```

## Implementation Notes

**CPU calculation** uses a stateful delta approach, storing previous idle and total tick counts to compute utilization over each interval, which avoids the misleading snapshots you get from single sample methods.

**Weather and location fetches** run on a dedicated thread with curl timeouts (5s for location, 10s for weather) so a slow network never stalls the UI. The render thread always reads a cached snapshot under a mutex.

**Static linking** (`MultiThreaded` MSVC runtime, `BUILD_SHARED_LIBS=OFF`) means the release binary is fully selfcontained with no runtime DLL dependencies.

**OS detection:** Initially used GetVersionEx for OS detection, but found it incorrectly reported Windows 11 as Windows 10 due to manifest shims; refactored to use ntdll.dll's RtlGetVersion for accuracy

## License

MIT
