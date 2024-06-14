// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC
#pragma once
#include "cosc/util.hpp" // this is used, but clang-tidy cannot detect it correctly
#include <glm/mat4x4.hpp>
#include <string>

namespace cosc {

/// An OpenGL shader wrapper.
/// Based on: https://learnopengl.com/Getting-started/Shaders
class Shader {
public:
    explicit Shader(const fs::path &vertexPath, const fs::path &fragmentPath);

    ~Shader();

    // TODO other constructors (copy constructor etc)

    void use();

    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, float value);
    void setMat4(const std::string &name, const glm::mat4 &value);
    void setMat3(const std::string &name, const glm::mat3 &value);
    void setVec3(const std::string &name, const glm::vec3 &value);

private:
    /// Shader program GL ID
    unsigned int shaderProgram;
};

}; // namespace cosc
