#include "cosc/animation.hpp"
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "cosc/lib/Simplex.h"
#include "cosc/util.hpp"
#include <glm/gtc/noise.hpp>
#include <glm/gtx/easing.hpp>

// Simplex noise parameters
constexpr float GAIN = 1.0f;
constexpr uint8_t OCTAVES = 4;
constexpr float LANCUNARITY = 1.7f;
constexpr float SCALE = 0.07f;

// FOV parameters
constexpr float MIN_FOV = 55.f;
constexpr float MAX_FOV = 65.f;

// Camera shake config, either shake the camera rotation, or shake FOV, or both
constexpr bool SHAKE_ORIENTATION = true;
constexpr bool SHAKE_FOV = false;
constexpr bool SCALE_CAMERA_MOVES = false;

constexpr float AXIS_X = 0.0;
constexpr float AXIS_Y = 1.0;
constexpr float AXIS_Z = 2.0;

// Allow spectral energy ratio to make animation half as slow, or half as fast
constexpr float ELAPSED_SCALE_MIN = 0.1;
constexpr float ELAPSED_SCALE_MAX = 10.0;

void cosc::CameraAnimationManager::update(float delta, float spectralEnergyRatio) {
    if (animations.empty()) {
        return;
    }
    // wrap animations around using modulo (basically loop them)
    if (nextAnimation) {
        curIdx++;
        SPDLOG_INFO("Advancing camera animation ({}/{})", curIdx, animations.size());
        elapsed = 0.f;
        nextAnimation = false;
    }
    const auto &anim = animations[curIdx % animations.size()];

    float progress = elapsed / anim.duration;
    if (progress >= 1.0f) {
        // advance to next animation
        nextAnimation = true;
        return;
    }

    // lerp position
    auto pos = glm::mix(anim.begin.pos, anim.end.pos, progress);

    // tweening... doesn't look as good as I was hoping for :(
    // auto pos = glm::vec3(x, y, z);
    // auto ease = glm::sineEaseOut(progress);
    // auto pos = glm::mix(anim.begin.pos, anim.end.pos, ease);

    // slerp angle
    auto orientation = glm::slerp(anim.begin.orientation, anim.end.orientation, progress);

    if constexpr (SHAKE_ORIENTATION) {
        // camera shake amount based on spectral energy
        auto scale = spectralEnergyRatio * SCALE;

        // camera shake based on Simplex noise under fractal Brownian motion.
        // note that the value is noise seed is based on the total time and axis
        auto shakeX = Simplex::fBm(glm::vec2 { total, AXIS_X }, OCTAVES, LANCUNARITY, GAIN) * scale;
        auto shakeY = Simplex::fBm(glm::vec2 { total, AXIS_Y }, OCTAVES, LANCUNARITY, GAIN) * scale;
        auto shakeZ = Simplex::fBm(glm::vec2 { total, AXIS_Z }, OCTAVES, LANCUNARITY, GAIN) * scale;

        // apply camera shake
        orientation = glm::rotate(orientation, shakeX, glm::vec3(1.0, 0.0, 0.0));
        orientation = glm::rotate(orientation, shakeY, glm::vec3(0.0, 1.0, 0.0));
        orientation = glm::rotate(orientation, shakeZ, glm::vec3(0.0, 0.0, 1.0));
    }

    camera.setEyePoint(pos);
    camera.setOrientation(orientation);

    if constexpr (SHAKE_FOV) {
        // apply FOV
        auto fov = cosc::util::mapRange(0.0, 1.0, MIN_FOV, MAX_FOV, spectralEnergyRatio);
        camera.setFov(static_cast<float>(fov));
    }

    // optionally scale movement amount using the spectral energy - scale within bounds
    // map spectral energy ratio to a multiplier that can be used to scale the delta time
    // this gives the visualiser that stutter-y "stop and go" look that some on YouTube also have
    if constexpr (SCALE_CAMERA_MOVES) {
        double deltaScalar
            = cosc::util::mapRange(0.0, 1.0, ELAPSED_SCALE_MIN, ELAPSED_SCALE_MAX, spectralEnergyRatio);
        elapsed += static_cast<float>(deltaScalar) * delta;
    } else {
        elapsed += delta;
    }

    // total is unaffected since it's used for noise
    total += delta;
}
