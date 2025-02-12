// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC
#include "cosc/shader.hpp"
#include "cosc/util.hpp"
#include "glad/gl.h"
#include <exception>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

cosc::Shader::Shader(const fs::path &vertexPath, const fs::path &fragmentPath) {
    SPDLOG_INFO("Loading shader. Vertex: {}, fragment: {}", vertexPath.string(), fragmentPath.string());

    // read shaders
    auto vertexSource = cosc::util::readPathToString(vertexPath);
    auto fragmentSource = cosc::util::readPathToString(fragmentPath);

    unsigned int vertexShader, fragmentShader;
    int success;
    char infoLog[512] = { 0 };

    // these have to be separate variables so we can make a pointer to them in glCreateShader
    // otherwise we could skip this (there might still be a way to skip)
    const char *vertexShaderStr = vertexSource.c_str();
    const char *fragmentShaderStr = fragmentSource.c_str();
    SPDLOG_TRACE("Instantiating a shader.\nVertex:\n{}\nFragment:\n{}", vertexSource, fragmentSource);

    // setup vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderStr, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        SPDLOG_ERROR("Failed to compile vertex shader!\n{}", infoLog);
        throw std::exception();
    }

    // setup fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderStr, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        SPDLOG_ERROR("Failed to compile frag shader!\n{}", infoLog);
        throw std::exception();
    }

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        SPDLOG_ERROR("Failed to link shaders!\n{}", infoLog);
        throw std::exception();
    }

    // free unused shader memory
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void cosc::Shader::use() {
    glUseProgram(shaderProgram);
}

void cosc::Shader::setBool(const std::string &name, bool value) {
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), (int) value);
}

void cosc::Shader::setInt(const std::string &name, int value) {
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void cosc::Shader::setFloat(const std::string &name, float value) {
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void cosc::Shader::setMat4(const std::string &name, const glm::mat4 &value) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void cosc::Shader::setMat3(const std::string &name, const glm::mat3 &value) {
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void cosc::Shader::setVec3(const std::string &name, const glm::vec3 &value) {
    glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, glm::value_ptr(value));
}

cosc::Shader::~Shader() {
    // NOTE: This may be incorrect depending on where the cosc::Shader destructor is called.
    // If GL errors occur this is a likely culprit.
    glDeleteProgram(shaderProgram);
}
