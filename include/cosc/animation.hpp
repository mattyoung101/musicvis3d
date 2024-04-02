#pragma once
#include "cosc/camera.hpp"
#include <vector>
namespace cosc {

/// A camera animation
class CameraAnimation {
public:
    /// Beginning camera pose
    CameraPose begin;
    /// Ending camera pose
    CameraPose end;
    // Duration of the animation in seconds
    float duration;

    explicit CameraAnimation(const CameraPose &poseBegin, const CameraPose &poseEnd, float time) :
        begin(poseBegin), end(poseEnd), duration(time) {};
};

/// Manages a camera with a set of animations.
class CameraAnimationManager {
public:
    explicit CameraAnimationManager(Camera &camera) : camera(camera) {};

    void addAnimation(const CameraAnimation &animation) {
        animations.emplace_back(animation);
    }

    void update(float delta);

private:
    Camera &camera;
    std::vector<CameraAnimation> animations{};
    /// True when we are advancing animations
    bool nextAnimation = true;
    /// Elapsed time in this animation
    float elapsed;
    /// Current animation index
    int curIdx = -1; // it will be incremented to 0 on the first run this way, kinda hacky
};

}
