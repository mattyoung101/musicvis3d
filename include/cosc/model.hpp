#pragma once
#include <filesystem>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "cosc/mesh.hpp"
#include "cosc/shader.hpp"
#include <glm/mat4x4.hpp>

namespace fs = std::filesystem;

namespace cosc {

/// A wrapper around a model loaded using the Assimp library.
/// Based on: https://learnopengl.com/Model-Loading/Model
class Model {
public:
    explicit Model(const fs::path &path);
    explicit Model(const std::vector<cosc::Mesh> &meshes)
        : meshes(meshes) {};

    /// Draws the model using the specified shader program.
    void draw(Shader &shader);

    /// Model transform matrix. By default, the identity matrix.
    glm::mat4 transform { 1.f };

    /// Inverse transform matrix for shader.
    glm::mat4 transformInv { 1.f };

    /// Model name or path.
    std::string name;

    /// Current yaw
    float yaw = 0.f;

    /// Current scale
    glm::vec3 scale{0.f, 0.f, 0.f};

    /// Current position
    glm::vec3 pos { 0.f, 0.f, 0.f };

    /// After adjusting yaw, scale and pos, applies these transforms to the transform matrix.
    /// Resets the transform to the identity first then applies translate, rotate and scale.
    /// (for minor project) only
    void applyTransform();

private:
    std::vector<Mesh> meshes {};
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};

};
