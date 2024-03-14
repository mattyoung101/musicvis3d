#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "cosc/shader.hpp"
#include "cosc/vertex.hpp"

namespace cosc {

/// A wrapper around a series of vertices, a mesh.
/// Based on: https://learnopengl.com/Model-Loading/Mesh
class Mesh {
public:
    std::vector<Vertex> verts {};
    std::vector<unsigned int> indices {};

    explicit Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
        : verts(vertices)
        , indices(indices) {
        setupMesh();
    }

    void draw(const cosc::Shader &shader) const;

private:
    unsigned int vao, vbo, ebo;
    void setupMesh();
};

};
