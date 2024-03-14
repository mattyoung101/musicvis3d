#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace cosc {
/// A vertex used in model loading with position and normal. We ignore tex coords since there are no textures
/// in this demo.
struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
};
}
