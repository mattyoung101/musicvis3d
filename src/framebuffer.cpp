#include "cosc/framebuffer.hpp"
#include "glad/gl.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

// clang-format off
// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
static const float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};
// clang-format on

// Based on: https://learnopengl.com/Advanced-OpenGL/Framebuffers

cosc::FrameBuffer::FrameBuffer(const fs::path &dataDir, const std::string &postShader, int width, int height)
    : quadShader(cosc::Shader(dataDir / "quad.vert.glsl", dataDir / postShader)) {
    SPDLOG_INFO("Initialising FrameBuffer");

    // generate quad mesh
    SPDLOG_DEBUG("Generating framebuffer quad");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    // allocate FBO
    SPDLOG_DEBUG("Allocating and generating FBO");
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // attach colour texture
    glGenTextures(1, &textureColour);
    glBindTexture(GL_TEXTURE_2D, textureColour);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // clamp texture to avoid artefacts in post-processing, mainly chromatic aberration
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColour, 0);

    // attach depth and stencil textures
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("GL says framebuffer is incomplete!");
    }
    SPDLOG_DEBUG("Framebuffer is complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cosc::FrameBuffer::bind() {
    if (bound) {
        throw std::runtime_error("Framebuffer is already bound, cannot bind again");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    bound = true;
}

void cosc::FrameBuffer::draw(float spectralEnergyRatio) {
    if (!bound) {
        throw std::runtime_error("Framebuffer is not bound but tried drawing");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    bound = false;

    // clear screen
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // use shader to draw
    quadShader.use();
    quadShader.setFloat("spectralEnergyRatio", spectralEnergyRatio);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, textureColour);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}
