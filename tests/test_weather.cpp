#include <gtest/gtest.h>
#include "system_stats.h"

TEST(WeatherData, DefaultLoadedIsFalse) {
    WeatherData w;
    EXPECT_FALSE(w.loaded);
}

TEST(WeatherData, DefaultTempIsZero) {
    WeatherData w;
    EXPECT_DOUBLE_EQ(w.tempF, 0.0);
}

TEST(WeatherData, DefaultConditionIsNotEmpty) {
    WeatherData w;
    EXPECT_FALSE(w.condition.empty());
}

TEST(LocationData, DefaultLoadedIsFalse) {
    LocationData loc;
    EXPECT_FALSE(loc.loaded);
}

TEST(LocationData, DefaultCoordinatesAreAtlanta) {
    LocationData loc;
    EXPECT_DOUBLE_EQ(loc.lat, 33.749);
    EXPECT_DOUBLE_EQ(loc.lon, -84.388);
}

TEST(LocationData, DefaultCityIsNotEmpty) {
    LocationData loc;
    EXPECT_FALSE(loc.city.empty());
}

// network test/ can be skipped if offline
TEST(FetchWeather, ReturnsStructWithValidCoords) {
    WeatherData w = FetchWeather(33.749, -84.388);
    EXPECT_GE(w.tempF, -200.0);
    EXPECT_LE(w.tempF, 200.0);
    EXPECT_FALSE(w.condition.empty());
    EXPECT_FALSE(w.icon.empty());
}

TEST(FetchWeather, WhenLoadedTempIsPlausible) {
    WeatherData w = FetchWeather(33.749, -84.388);
    if (!w.loaded) GTEST_SKIP() << "Network unavailable";
    EXPECT_GT(w.tempF, -100.0);
    EXPECT_LT(w.tempF, 150.0);
}

TEST(FetchWeather, WhenLoadedConditionIsNonEmpty) {
    WeatherData w = FetchWeather(33.749, -84.388);
    if (!w.loaded) GTEST_SKIP() << "Network unavailable";
    EXPECT_FALSE(w.condition.empty());
    EXPECT_FALSE(w.icon.empty());
}

TEST(FetchWeather, ExtremeLatLonDoesNotCrash) {
    WeatherData w1 = FetchWeather(90.0, 0.0);
    WeatherData w2 = FetchWeather(-90.0, 180.0);
    EXPECT_FALSE(w1.condition.empty());
    EXPECT_FALSE(w2.condition.empty());
}

TEST(FetchLocation, ReturnsStruct) {
    LocationData loc = FetchLocation();
    // Must always return a valid struct
    EXPECT_FALSE(loc.city.empty());
}

TEST(FetchLocation, WhenLoadedCoordinatesAreInRange) {
    LocationData loc = FetchLocation();
    if (!loc.loaded) GTEST_SKIP() << "Network unavailable";
    EXPECT_GE(loc.lat, -90.0);
    EXPECT_LE(loc.lat,  90.0);
    EXPECT_GE(loc.lon, -180.0);
    EXPECT_LE(loc.lon,  180.0);
}

TEST(FetchLocation, WhenLoadedCityIsNonEmpty) {
    LocationData loc = FetchLocation();
    if (!loc.loaded) GTEST_SKIP() << "Network unavailable";
    EXPECT_FALSE(loc.city.empty());
}