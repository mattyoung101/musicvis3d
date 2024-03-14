#pragma once
#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cosc {


typedef enum {
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT
} MovementType_t;

/// A simple perspective projection camera.
/// Based on: https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h
class Camera {
public:
    glm::vec3 pos;
    glm::vec3 front{0.f, 0.f, -1.f};
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float zoom = 45.f;
    float speed = 2.5f;
    float sensitivity = 0.1f;
    float nearClip = 0.1f; // near clipping plane
    float farClip = 100.f; // far clipping plane

    Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
        float pitch = 0.f, float yaw = -90.f)
        : pos(position)
        , worldUp(up)
        , yaw(yaw)
        , pitch(pitch) {
        updateCameraVectors();
    }
    
    /// Computes the view matrix. Should be called each frame.
    glm::mat4 viewMatrix() {
        return glm::lookAt(pos, pos + front, up);
    }
    
    /// Computes the projection matrix. Should be called each frame.
    glm::mat4 projectionMatrix(float screenWidth, float screenHeight) {
        return glm::perspective(glm::radians(zoom), screenWidth / screenHeight, nearClip, farClip);
    }

    void processKeyboardInput(MovementType_t dir, float delta) {
        float vel = speed * delta;
        if (dir == MOVE_FORWARD) {
            pos += front * vel;
        } else if (dir == MOVE_BACKWARD) {
            pos -= front * vel;
        } else if (dir == MOVE_LEFT) {
            pos -= right * vel;
        } else if (dir == MOVE_RIGHT) {
            pos += right * vel;
        }
    }

    void processMouseInput(float deltaX, float deltaY) {
        yaw += deltaX * sensitivity;
        pitch += deltaY * sensitivity;
        pitch = std::clamp(pitch, -89.f, 89.f);
        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        // calculate new front vector
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw) * cos(glm::radians(pitch)));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw) * cos(glm::radians(pitch)));
        front = glm::normalize(newFront);

        // calculate new right and up
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

};
