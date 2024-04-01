#pragma once
#include "cosc/camera.hpp"
#include <cstddef>
#include <vector>
namespace cosc {

/// A camera animation
class CameraAnimation {
public:
    /// Beginning camera pose
    CameraPose poseBegin;
    /// Ending camera pose
    CameraPose poseEnd;
    /// Interpolation time in seconds
    float timeSeconds;

    explicit CameraAnimation(const CameraPose &poseBegin, const CameraPose &poseEnd, float time) :
        poseBegin(poseBegin), poseEnd(poseEnd), timeSeconds(time) {};
};

/// Manages a camera with a set of animations.
class CameraAnimationManager {
public:
    explicit CameraAnimationManager(Camera &camera) : camera(camera) {};

    void addAnimation(const CameraAnimation &animation) {
        animations.emplace_back(animation);
    }

    void update(float delta);

    void setRunning(bool running);

private:
    Camera &camera;
    std::vector<CameraAnimation> animations{};
    bool running;
    size_t currentAnimation;
    float totalDelta;
};

}
