#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "cosc/shader.hpp"
#include "cosc/texture.hpp"
#include "cosc/vertex.hpp"

namespace cosc {

/// A wrapper around a series of vertices, a mesh.
/// Based on: https://learnopengl.com/Model-Loading/Mesh
class Mesh {
public:
    std::vector<Vertex> verts {};
    std::vector<unsigned int> indices {};
    std::vector<Texture> textures {};

    explicit Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
        : verts(vertices)
        , indices(indices)
        , textures(textures) {
        setupMesh();
    }

    void draw(cosc::Shader &shader) const;

private:
    unsigned int vao, vbo, ebo;
    void setupMesh();
};

};
