#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace cosc::util {

typedef std::chrono::time_point<std::chrono::steady_clock> SteadyTimePoint_t;
typedef std::chrono::duration<double, std::nano> NanoDuration_t;

/// Reads the contents of path to a string.
std::string readPathToString(const fs::path &path);

/** Interpolates between a and b. f must be in [0, 1] */
constexpr double lerp(double a, double b, double f) {
    // Source: https://stackoverflow.com/q/4353525/5007892
    return (a * (1.0 - f)) + (b * f);
}

};
