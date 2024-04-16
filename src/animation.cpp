#include "cosc/animation.hpp"
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/easing.hpp>
#include <glm/gtc/noise.hpp>

constexpr float SHAKE_SCALE = 0.5f;

void cosc::CameraAnimationManager::update(float delta) {
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
    //auto pos = glm::vec3(x, y, z);
    // auto ease = glm::sineEaseOut(progress);
    // auto pos = glm::mix(anim.begin.pos, anim.end.pos, ease);

    // slerp angle
    auto orientation = glm::slerp(anim.begin.orientation, anim.end.orientation, progress);

    // camera shake based on simplex noise, one value per axis, based on the time
    auto shakeX = glm::simplex(glm::vec2{total, 0.0}) * SHAKE_SCALE;
    auto shakeY = glm::simplex(glm::vec2{total, 1.0}) * SHAKE_SCALE;
    auto shakeZ = glm::simplex(glm::vec2{total, 2.0}) * SHAKE_SCALE;

    // apply camera shake
    orientation = glm::rotate(orientation, shakeX, glm::vec3(1.0, 0.0, 0.0));
    orientation = glm::rotate(orientation, shakeY, glm::vec3(0.0, 1.0, 0.0));
    orientation = glm::rotate(orientation, shakeZ, glm::vec3(0.0, 0.0, 1.0));

    camera.setEyePoint(pos);
    camera.setOrientation(orientation);

    elapsed += delta;
    total += delta;
}
