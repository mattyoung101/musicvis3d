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

/// If true, skip intro
#define SKIP_INTRO 0

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
constexpr float INTRO_SLIDE_TIME = 3.0;

/// Number of intro slides
constexpr size_t INTRO_NUM_SLIDES = 3;

namespace cosc {
enum class AppStatus {
    INTRO = 1,
    RUNNING = 2,
    QUIT = 3
};

constexpr bool isAppRunning(AppStatus status) {
    return status != AppStatus::QUIT;
}

constexpr bool isNotInIntro(AppStatus status) {
    return status >= AppStatus::RUNNING;
}

constexpr bool isInIntro(AppStatus status) {
    return status == AppStatus::INTRO;
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
