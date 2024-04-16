#include "cosc/animation.hpp"
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/easing.hpp>
#include <glm/gtc/noise.hpp>
#include "cosc/lib/Simplex.h"
#include "cosc/util.hpp"

// Simplex noise parameters
constexpr float GAIN = 1.0f;
constexpr uint8_t OCTAVES = 4;
constexpr float LANCUNARITY = 1.7f;
constexpr float SCALE = 0.1f;

// FOV parameters
constexpr float MIN_FOV = 55.f;
constexpr float MAX_FOV = 65.f;

// Camera shake config, either shake the camera rotation, or shake FOV, or both
constexpr bool SHAKE_ORIENTATION = true;
constexpr bool SHAKE_FOV = false;

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
        auto shakeX = Simplex::fBm(glm::vec2 { total, 0.0 }, OCTAVES, LANCUNARITY, GAIN) * scale;
        auto shakeY = Simplex::fBm(glm::vec2 { total, 1.0 }, OCTAVES, LANCUNARITY, GAIN) * scale;
        auto shakeZ = Simplex::fBm(glm::vec2 { total, 2.0 }, OCTAVES, LANCUNARITY, GAIN) * scale;

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
        camera.setFov(fov);
    }

    elapsed += delta;
    total += delta;
}
