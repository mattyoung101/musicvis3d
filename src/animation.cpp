#include "cosc/animation.hpp"
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/easing.hpp>

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
    camera.setEyePoint(pos);
    camera.setOrientation(orientation);

    elapsed += delta;
}
