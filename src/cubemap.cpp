#include "cosc/cubemap.hpp"
#include "cosc/shader.hpp"
#include "glad/gl.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include "cosc/lib/stb_image.h"
#include <glm/mat3x3.hpp>

// clang-format off
constexpr float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
// clang-format on

cosc::Cubemap::Cubemap(const fs::path &dataDir, const fs::path &cubeMapDir)
    : shader(Shader(dataDir / "cubemap.vert.glsl", dataDir / "cubemap.frag.glsl")) {
    SPDLOG_INFO("Instantiating Cubemap");

    // create mesh
    SPDLOG_INFO("Generating skybox mesh data");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    // generate and load OpenGL textures
    SPDLOG_INFO("Loading cubemap textures");
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    std::vector<std::string> faces = { "right", "left", "top", "bottom", "front", "back" };
    int width, height, channels;
    unsigned char *data;
    int i = 0;
    for (const auto &face : faces) {
        auto facePath = dataDir / cubeMapDir / (face + ".jpg");

        // load data from FS
        SPDLOG_DEBUG("Loading face: {} from path: {}", face, facePath.string());
        data = stbi_load(facePath.c_str(), &width, &height, &channels, 0);
        if (data == nullptr) {
            throw std::runtime_error("Failed to decode cubemap face!");
        }
        SPDLOG_DEBUG("Retrieved a {}x{} image with {} channels", width, height, channels);

        // submit to OpenGL
        // FIXME ideally make this work with PNG RGBA - it's borked, hence we moved to JPEG
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (i++), 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        // has been copied to the GPU presumably, so we can free it here
        stbi_image_free(data);

        // apply filtering
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    // configure the shader
    // TODO what is going on here?
    shader.use();
    shader.setInt("skybox", 0);
}

void cosc::Cubemap::draw(const Camera &camera) {
    // when drawing last, we change the depth test so that it passes when values are <= buffer content
    // TODO what does this really do
    glDepthFunc(GL_LEQUAL);
    shader.use();

    // remove translation from the view matrix - similar to what we do in basic lighting
    auto view = glm::mat4(glm::mat3(camera.getViewMatrix()));
    shader.setMat4("view", view);
    shader.setMat4("projection", camera.getProjectionMatrix());

    // draw skybox cube
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // restore original depth function
    glDepthFunc(GL_LESS);
}

// FIXME dispose resources
