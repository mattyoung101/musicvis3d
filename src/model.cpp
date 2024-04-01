#include "cosc/model.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include "cosc/texture.hpp"
#include "glad/gl.h"
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

cosc::Model::Model(const fs::path &path) {
    SPDLOG_INFO("Loading model {}", path.string());
    name = path.string();

    Assimp::Importer importer {};
    const aiScene *scene
        = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SPDLOG_ERROR(
            "Failed to import model '{}' using Assimp: {}", path.string(), importer.GetErrorString());
        return;
    }

    processNode(scene->mRootNode, scene);
}

void cosc::Model::processNode(aiNode *node, const aiScene *scene) {
    SPDLOG_DEBUG("Process node {}", node->mName.C_Str());

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

cosc::Mesh cosc::Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.norm = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        vertices.push_back(vertex);
    }

    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process material
    if (mesh->mMaterialIndex >= 0) {
        // TODO
    }

    return Mesh(vertices, indices, textures);
}

void cosc::Model::draw(Shader &shader) {
    // send transform matrix to vertex shader
    // source: https://learnopengl.com/Getting-started/Coordinate-Systems (section "Going 3D")
    shader.setMat4("model", transform);

    // compute inverse transform and supply to shader
    // this is because, as noted in LearnOpenGL, matrix inversion is expensive for shaders and we can avoid a
    // per-vertex computation by computing the transform here.
    transformInv = glm::mat3(glm::transpose(glm::inverse(transform)));
    shader.setMat3("modelInv", transformInv);

    for (const auto &mesh : meshes) {
        mesh.draw(shader);
    }
}

void cosc::Model::applyTransform() {
    // reset transform matrix
    transform = glm::mat4{1.f};

    // based on the matrices described in: https://learnopengl.com/Getting-started/Transformations

    // apply scale (S_x, S_y, S_z on the diagonal)
    glm::mat4 scaleMat{1.f};
    scaleMat = glm::scale(scaleMat, scale);

    // apply translate (T_x, T_y, T_z on the final column)
    glm::mat4 translateMat{1.f};
    translateMat = glm::translate(translateMat, glm::vec3(pos.x, pos.y, pos.z));

    // apply rotation (yaw only)
    glm::mat4 rotateMat{1.f};
    // convert yaw in degrees to radians
    float yawRad = glm::radians(yaw);
    rotateMat = glm::rotate(rotateMat, yawRad, glm::vec3{0.f, 1.f, 0.f});

    // apply final transform by multiplying matrix
    // this is treated in the order: rotate, translate, scale
    transform = scaleMat * translateMat * rotateMat;
}
