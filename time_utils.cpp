#include <chrono>
#include <format>

struct TimeSnapshot {
    std::string est, pst, gmt, eet, utc8;
};

TimeSnapshot GetCurrentTimes() {
    auto now = std::chrono::utc_clock::now();
    auto nowSys = std::chrono::utc_clock::to_sys(now);
    auto toHHMM = [](auto tp) {
        auto t = std::chrono::system_clock::to_time_t(tp);
        std::tm tm{};
        gmtime_s(&tm, &t);
        return std::format("{:02}:{:02}", tm.tm_hour, tm.tm_min);
    };

    using namespace std::chrono_literals;
    return {
        toHHMM(nowSys - 4h),   // EST  UTC-5
        toHHMM(nowSys - 7h),   // PST  UTC-8
        toHHMM(nowSys + 0h),   // GMT  UTC+0
        toHHMM(nowSys + 2h),   // EET  UTC+2
        toHHMM(nowSys + 8h),   // UTC+8
    };
}