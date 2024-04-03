#include "cosc/animation.hpp"
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>
#include "cosc/lib/tweeny-3.2.0.h"

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
    // slerp angle
    auto orientation = glm::slerp(anim.begin.orientation, anim.end.orientation, progress);
    camera.setEyePoint(pos);
    camera.setOrientation(orientation);

    elapsed += delta;
}
