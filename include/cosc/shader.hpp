#pragma once
#include <string>
#include <glm/mat4x4.hpp>

namespace cosc {

/// An OpenGL shader wrapper.
/// Based on: https://learnopengl.com/Getting-started/Shaders
class Shader {
public:
    /// Shader program GL ID
    unsigned int shaderProgram;

    explicit Shader(const std::string &vertexPath, const std::string &fragmentPath);

    ~Shader();

    void use();

    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, float value);
    void setMat4(const std::string &name, const glm::mat4 &value);
    void setMat3(const std::string &name, const glm::mat3 &value);
    void setVec3(const std::string &name, const glm::vec3 &value);
};

};
