#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Project defines.

/// If true, mute audio (mostly for debugging so I don't get sick of the songs!)
#define MUTE 0

/// If true, run the app in fullscreen
#define FULLSCREEN 1

/// If true, do wireframe rendering
#define WIREFRAME 0

/// Units between bars
constexpr float BAR_SPACING = 2.5;

/// Scaling factor for the bars
constexpr float BAR_SCALING = 0.1;

/// Bar width multiplier - applied after BAR_SCALING
constexpr float BAR_WIDTH_MULT = 2.0;

/// Min height for an extended bar (multiplied by BAR_SCALING)
constexpr float BAR_MIN_HEIGHT = 1.0;

/// Max height for an extended bar (multiplied by BAR_SCALING)
constexpr float BAR_MAX_HEIGHT = 50.;

/// Intro slide time in seconds
constexpr float INTRO_SLIDE_TIME = 1.5;

namespace cosc {
enum class AppStatus {
    INTRO_SLIDE1 = 0,
    INTRO_SLIDE2 = 1,
    INTRO_SLIDE3 = 2,
    RUNNING = 3,
    QUIT = 4
};

constexpr bool isAppRunning(AppStatus status) {
    return status != AppStatus::QUIT;
}

constexpr bool isNotInIntro(AppStatus status) {
    return status >= AppStatus::RUNNING;
}

};

namespace cosc::util {

/// Reads the contents of path to a string.
std::string readPathToString(const fs::path &path);

// Source: https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/math/MathUtils.java#L385
// Apache 2.0
constexpr double mapRange(
    double inRangeStart, double inRangeEnd, double outRangeStart, double outRangeEnd, double value) {
    return outRangeStart
        + (value - inRangeStart) * (outRangeEnd - outRangeStart) / (inRangeEnd - inRangeStart);
}

};
