#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace cosc {
/// A vertex used in model loading with position and normal.
struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 texCoords;
};
}
