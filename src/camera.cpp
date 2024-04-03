#include "cosc/camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace cosc;
using namespace glm;

constexpr float BOOST_SCALE = 3.0f;
constexpr float SPEED = 2.5f;
constexpr float POINTER_SPEED = 1.6f;

// https://github.com/cinder/Cinder/blob/093ea5f81523e7fc528c5787a48444f037e6b08f/src/cinder/Matrix.cpp#L134
inline glm::mat4 alignZAxisWithTarget(vec3 targetDir, vec3 upDir) {
    // Ensure that the target direction is non-zero.
    if (length2(targetDir) == 0)
        targetDir = vec3(0, 0, 1);

    // Ensure that the up direction is non-zero.
    if (length2(upDir) == 0)
        upDir = vec3(0, 1, 0);

    // Check for degeneracies.  If the upDir and targetDir are parallel
    // or opposite, then compute a new, arbitrary up direction that is
    // not parallel or opposite to the targetDir.
    if (length2(cross(upDir, targetDir)) == 0) {
        upDir = cross(targetDir, vec3(1, 0, 0));
        if (length2(upDir) == 0)
            upDir = cross(targetDir, vec3(0, 0, 1));
    }

    // Compute the x-, y-, and z-axis vectors of the new coordinate system.
    vec3 targetPerpDir = cross(upDir, targetDir);
    vec3 targetUpDir = cross(targetDir, targetPerpDir);

    // Rotate the x-axis into targetPerpDir (row 0),
    // rotate the y-axis into targetUpDir   (row 1),
    // rotate the z-axis into targetDir     (row 2).
    vec3 row[3];
    row[0] = normalize(targetPerpDir);
    row[1] = normalize(targetUpDir);
    row[2] = normalize(targetDir);

    const float v[16] = { row[0].x, row[0].y, row[0].z, 0, row[1].x, row[1].y, row[1].z, 0, row[2].x,
        row[2].y, row[2].z, 0, 0, 0, 0, 1 };

    return glm::make_mat4(v);
}

void Camera::setEyePoint(const vec3 &eyePoint) {
    mEyePoint = eyePoint;
    dirtyViewCaches();
}

void Camera::setViewDirection(const vec3 &viewDirection) {
    mViewDirection = normalize(viewDirection);
    mOrientation = glm::rotation(mViewDirection, glm::vec3(0.f, 0.f, -1.f));
    dirtyViewCaches();
}

void Camera::setOrientation(const quat &orientation) {
    mOrientation = glm::normalize(orientation);
    mViewDirection = glm::rotate(mOrientation, glm::vec3(0, 0, -1));
    dirtyViewCaches();
}

// Derived from math presented in http://paulbourke.net/miscellaneous/lens/
float Camera::getFocalLength() const {
    return 1.f / (std::tan(glm::radians(mFov) * 0.5f) * 2.f);
}

void Camera::setWorldUp(const vec3 &worldUp) {
    mWorldUp = normalize(worldUp);
    mOrientation = glm::toQuat(alignZAxisWithTarget(-mViewDirection, worldUp));
    dirtyViewCaches();
}

void Camera::lookAt(const vec3 &target) {
    mViewDirection = normalize(target - mEyePoint);
    mOrientation = glm::toQuat(alignZAxisWithTarget(-mViewDirection, mWorldUp));
    mPivotDistance = distance(target, mEyePoint);
    dirtyViewCaches();
}

void Camera::lookAt(const vec3 &eyePoint, const vec3 &target) {
    mEyePoint = eyePoint;
    mViewDirection = normalize(target - mEyePoint);
    mOrientation = quat(glm::toQuat(alignZAxisWithTarget(-mViewDirection, mWorldUp)));
    mPivotDistance = distance(target, mEyePoint);
    dirtyViewCaches();
}

void Camera::lookAt(const vec3 &eyePoint, const vec3 &target, const vec3 &aWorldUp) {
    mEyePoint = eyePoint;
    mWorldUp = normalize(aWorldUp);
    mViewDirection = normalize(target - mEyePoint);
    mOrientation = glm::toQuat(alignZAxisWithTarget(-mViewDirection, mWorldUp));
    mPivotDistance = distance(target, mEyePoint);
    dirtyViewCaches();
}

void Camera::getFrustum(float *left, float *top, float *right, float *bottom, float *near, float *far) const {
    calcMatrices();

    *left = mFrustumLeft;
    *top = mFrustumTop;
    *right = mFrustumRight;
    *bottom = mFrustumBottom;
    *near = mNearClip;
    *far = mFarClip;
}

void Camera::getBillboardVectors(vec3 *right, vec3 *up) const {
    *right = glm::vec3(glm::row(getViewMatrix(), 0));
    *up = glm::vec3(glm::row(getViewMatrix(), 1));
}

vec2 Camera::worldToScreen(const vec3 &worldCoord, float screenWidth, float screenHeight) const {
    vec4 eyeCoord = getViewMatrix() * vec4(worldCoord, 1);
    vec4 ndc = getProjectionMatrix() * eyeCoord;
    ndc.x /= ndc.w;
    ndc.y /= ndc.w;
    ndc.z /= ndc.w;

    return vec2((ndc.x + 1.0f) / 2.0f * screenWidth, (1.0f - (ndc.y + 1.0f) / 2.0f) * screenHeight);
}

vec2 Camera::eyeToScreen(const vec3 &eyeCoord, const vec2 &screenSizePixels) const {
    vec4 ndc = getProjectionMatrix() * vec4(eyeCoord, 1);
    ndc.x /= ndc.w;
    ndc.y /= ndc.w;
    ndc.z /= ndc.w;

    return vec2(
        (ndc.x + 1.0f) / 2.0f * screenSizePixels.x, (1.0f - (ndc.y + 1.0f) / 2.0f) * screenSizePixels.y);
}

float Camera::worldToEyeDepth(const vec3 &worldCoord) const {
    const mat4 &m = getViewMatrix();
    return m[0][2] * worldCoord.x + m[1][2] * worldCoord.y + m[2][2] * worldCoord.z + m[3][2];
}

vec3 Camera::worldToNdc(const vec3 &worldCoord) const {
    vec4 eye = getViewMatrix() * vec4(worldCoord, 1);
    vec4 unproj = getProjectionMatrix() * eye;
    return vec3(unproj.x / unproj.w, unproj.y / unproj.w, unproj.z / unproj.w);
}

void Camera::calcMatrices() const {
    if (!mModelViewCached)
        calcViewMatrix();
    if (!mProjectionCached)
        calcProjection();
}

void Camera::calcViewMatrix() const {
    mW = -normalize(mViewDirection);
    mU = glm::rotate(mOrientation, glm::vec3(1, 0, 0));
    mV = glm::rotate(mOrientation, glm::vec3(0, 1, 0));

    vec3 d(-dot(mEyePoint, mU), -dot(mEyePoint, mV), -dot(mEyePoint, mW));

    mat4 &m = mViewMatrix;
    m[0][0] = mU.x;
    m[1][0] = mU.y;
    m[2][0] = mU.z;
    m[3][0] = d.x;
    m[0][1] = mV.x;
    m[1][1] = mV.y;
    m[2][1] = mV.z;
    m[3][1] = d.y;
    m[0][2] = mW.x;
    m[1][2] = mW.y;
    m[2][2] = mW.z;
    m[3][2] = d.z;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;

    mModelViewCached = true;
    mInverseModelViewCached = false;
}

void Camera::calcInverseView() const {
    if (!mModelViewCached)
        calcViewMatrix();

    mInverseModelViewMatrix = glm::inverse(mViewMatrix);
    mInverseModelViewCached = true;
}

void Camera::getClipCoordinates(
    float clipDist, float ratio, vec3 *topLeft, vec3 *topRight, vec3 *bottomLeft, vec3 *bottomRight) const {
    calcMatrices();

    const vec3 viewDirection = normalize(mViewDirection);

    *topLeft
        = mEyePoint + clipDist * viewDirection + ratio * (mFrustumTop * mV) + ratio * (mFrustumLeft * mU);
    *topRight
        = mEyePoint + clipDist * viewDirection + ratio * (mFrustumTop * mV) + ratio * (mFrustumRight * mU);
    *bottomLeft
        = mEyePoint + clipDist * viewDirection + ratio * (mFrustumBottom * mV) + ratio * (mFrustumLeft * mU);
    *bottomRight
        = mEyePoint + clipDist * viewDirection + ratio * (mFrustumBottom * mV) + ratio * (mFrustumRight * mU);
}


// Based on my other code:
// https://github.com/DECO3801-Segfault-Coredump/hermes/blob/master/core/src/main/kotlin/com/decosegfault/atlas/util/FirstPersonCamController.kt
// Which in turn is based on ThreeJS pointer lock controls:
// - https://github.com/mrdoob/three.js/blob/dev/examples/jsm/controls/PointerLockControls.js
// - https://sbcode.net/threejs/pointerlock-controls/

void Camera::moveForward(float distance) {
    auto degrees90 = glm::radians(90.f);
    auto delta = glm::rotateY(glm::normalize(glm::cross(getViewDirection(), getWorldUp())), degrees90) * distance;
    mEyePoint += delta;
}

void Camera::moveRight(float distance) {
    auto delta = glm::normalize(glm::cross(getViewDirection(), getWorldUp())) * distance;
    mEyePoint += delta;
}

void Camera::processKeyboardInput(MovementType_t dir, float delta, bool boost) {
    float distance = SPEED * (boost ? BOOST_SCALE : 1.f) * delta;
    if (dir == MOVE_FORWARD) {
        moveForward(distance);
    } else if (dir == MOVE_BACKWARD) {
        moveForward(-distance);
    } else if (dir == MOVE_LEFT) {
        moveRight(-distance);
    } else if (dir == MOVE_RIGHT) {
        moveRight(distance);
    } else if (dir == MOVE_UP) {
        mEyePoint.y += distance;
    } else if (dir == MOVE_DOWN) {
        mEyePoint.y -= distance;
    }
    dirtyViewCaches();
}

void Camera::processMouseInput(float deltaX, float deltaY) {
    // compute Euler angles
    auto pitch = glm::degrees(glm::pitch(getOrientation()));
    auto yaw = glm::degrees(glm::yaw(getOrientation()));
    //SPDLOG_DEBUG("before pitch: {}, yaw: {}", pitch, yaw);

    // rotate
    pitch += deltaY * 0.1f * POINTER_SPEED;
    yaw += -deltaX * 0.1f * POINTER_SPEED;

    // clamp pitch -89 degrees to 89 degrees
    pitch = std::clamp(pitch, -89.f, 89.f);
    // FIXME this shouldn't be required but otherwise it explodes
    yaw = std::clamp(yaw, -89.f, 89.f);
    //SPDLOG_DEBUG("after pitch: {}, yaw: {}", pitch, yaw);

    // update quaternion
    // order is: pitch, yaw, roll https://gamedev.stackexchange.com/a/13441/72826
    auto newQuat = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.f));
    setOrientation(newQuat);
}

////////////////////////////////////////////////////////////////////////////////////////
// CameraPersp
// Creates a default camera resembling Maya Persp
CameraPersp::CameraPersp() {
    lookAt(vec3(28, 21, 28), vec3(), vec3(0, 1, 0));
    setPerspective(35, 1.3333f, 0.1f, 1000);
}

CameraPersp::CameraPersp(int pixelWidth, int pixelHeight, float fovDegrees) {
    float eyeX = pixelWidth / 2.0f;
    float eyeY = pixelHeight / 2.0f;
    float halfFov = 3.14159f * fovDegrees / 360.0f;
    float theTan = std::tan(halfFov);
    float dist = eyeY / theTan;
    float nearDist = dist / 10.0f; // near / far clip plane
    float farDist = dist * 10.0f;
    float aspect = pixelWidth / (float) pixelHeight;

    setPerspective(fovDegrees, aspect, nearDist, farDist);
    lookAt(vec3(eyeX, eyeY, dist), vec3(eyeX, eyeY, 0.0f));
}

CameraPersp::CameraPersp(int pixelWidth, int pixelHeight, float fovDegrees, float nearPlane, float farPlane) {
    float halfFov, theTan, aspect;

    float eyeX = pixelWidth / 2.0f;
    float eyeY = pixelHeight / 2.0f;
    halfFov = 3.14159f * fovDegrees / 360.0f;
    theTan = std::tan(halfFov);
    float dist = eyeY / theTan;
    aspect = pixelWidth / (float) pixelHeight;

    setPerspective(fovDegrees, aspect, nearPlane, farPlane);
    lookAt(vec3(eyeX, eyeY, dist), vec3(eyeX, eyeY, 0.0f));
}

void CameraPersp::setPerspective(
    float verticalFovDegrees, float aspectRatio, float nearPlane, float farPlane) {
    mFov = verticalFovDegrees;
    mAspectRatio = aspectRatio;
    mNearClip = nearPlane;
    mFarClip = farPlane;

    mProjectionCached = false;
}

void CameraPersp::calcProjection() const {
    mFrustumTop = mNearClip * std::tan((float) M_PI / 180.0f * mFov * 0.5f);
    mFrustumBottom = -mFrustumTop;
    mFrustumRight = mFrustumTop * mAspectRatio;
    mFrustumLeft = -mFrustumRight;

    // perform lens shift
    // if (mLensShift.y != 0.0f) {
    //     mFrustumTop = ci::lerp<float, float>(0.0f, 2.0f * mFrustumTop, 0.5f + 0.5f * mLensShift.y);
    //     mFrustumBottom = ci::lerp<float, float>(2.0f * mFrustumBottom, 0.0f, 0.5f + 0.5f * mLensShift.y);
    // }
    //
    // if (mLensShift.x != 0.0f) {
    //     mFrustumRight = ci::lerp<float, float>(2.0f * mFrustumRight, 0.0f, 0.5f - 0.5f * mLensShift.x);
    //     mFrustumLeft = ci::lerp<float, float>(0.0f, 2.0f * mFrustumLeft, 0.5f - 0.5f * mLensShift.x);
    // }

    mat4 &p = mProjectionMatrix;
    p[0][0] = 2.0f * mNearClip / (mFrustumRight - mFrustumLeft);
    p[1][0] = 0.0f;
    p[2][0] = (mFrustumRight + mFrustumLeft) / (mFrustumRight - mFrustumLeft);
    p[3][0] = 0.0f;

    p[0][1] = 0.0f;
    p[1][1] = 2.0f * mNearClip / (mFrustumTop - mFrustumBottom);
    p[2][1] = (mFrustumTop + mFrustumBottom) / (mFrustumTop - mFrustumBottom);
    p[3][1] = 0.0f;

    p[0][2] = 0.0f;
    p[1][2] = 0.0f;
    p[2][2] = -(mFarClip + mNearClip) / (mFarClip - mNearClip);
    p[3][2] = -2.0f * mFarClip * mNearClip / (mFarClip - mNearClip);

    p[0][3] = 0.0f;
    p[1][3] = 0.0f;
    p[2][3] = -1.0f;
    p[3][3] = 0.0f;

    mat4 &m = mInverseProjectionMatrix;
    m[0][0] = (mFrustumRight - mFrustumLeft) / (2.0f * mNearClip);
    m[1][0] = 0.0f;
    m[2][0] = 0.0f;
    m[3][0] = (mFrustumRight + mFrustumLeft) / (2.0f * mNearClip);

    m[0][1] = 0.0f;
    m[1][1] = (mFrustumTop - mFrustumBottom) / (2.0f * mNearClip);
    m[2][1] = 0.0f;
    m[3][1] = (mFrustumTop + mFrustumBottom) / (2.0f * mNearClip);

    m[0][2] = 0.0f;
    m[1][2] = 0.0f;
    m[2][2] = 0.0f;
    m[3][2] = -1.0f;

    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = -(mFarClip - mNearClip) / (2.0f * mFarClip * mNearClip);
    m[3][3] = (mFarClip + mNearClip) / (2.0f * mFarClip * mNearClip);

    mProjectionCached = true;
}

void CameraPersp::setLensShift(float horizontal, float vertical) {
    mLensShift.x = horizontal;
    mLensShift.y = vertical;

    mProjectionCached = false;
}

CameraPersp CameraPersp::subdivide(const glm::uvec2 &gridSize, const glm::uvec2 &gridIndex) const {
    CameraPersp result = *this;
    result.setAspectRatio(getAspectRatio() * float(gridSize.x) / float(gridSize.y));
    result.setLensShift(vec2(1.0f) - vec2(gridSize.y, gridSize.x) + 2.0f * vec2(gridIndex.y, gridIndex.x));
    return result;
}
