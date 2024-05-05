#include "cosc/intro.hpp"
#include <spdlog/spdlog.h>
#include "glad/gl.h"
#include "cosc/lib/stb_image.h"

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


// Quad rendering based on: https://learnopengl.com/Advanced-OpenGL/Framebuffers
// Texture loading based on: https://learnopengl.com/Getting-started/Textures

cosc::IntroManager::IntroManager(const fs::path &dataDir)
    : shader(cosc::Shader(dataDir / "quad.vert.glsl", dataDir / "quad.frag.glsl")) {
    SPDLOG_INFO("Initialising IntroManager");

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *data = nullptr;

    SPDLOG_DEBUG("Generating intro quad mesh data");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    SPDLOG_DEBUG("Loading intro textures");
    stbi_set_flip_vertically_on_load(true);

    for (size_t i = 0; i < INTRO_NUM_SLIDES; i++) {
        auto path = dataDir / ("slide" + std::to_string(i) + ".png");
        // auto path = dataDir / "intro_debug.png";
        SPDLOG_DEBUG("Loading slide: {}", path.string());
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (data == nullptr) {
            throw std::runtime_error("Failed to decode image!");
        }
        SPDLOG_DEBUG("Retrieved a {}x{} image with {} channels", width, height, channels);

        // generate a GL texture id
        unsigned int texId = 0;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        // submit to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // has been copied to the GPU presumably, so we can free it here
        stbi_image_free(data);
        SPDLOG_DEBUG("Allocated texture id {}", texId);

        textureIds.push_back(texId);
    }

    stbi_set_flip_vertically_on_load(false); // back to default setting
}

void cosc::IntroManager::draw(size_t slideNumber) {
    if (slideNumber > textureIds.size() - 1) {
        SPDLOG_WARN("Requested slide number {} >= textures array size {} - will not draw!", slideNumber,
            textureIds.size());
        return;
    }

    // SPDLOG_DEBUG("Drawing slide number {} with texture id {}", slideNumber, textureIds[slideNumber]);

    // Don't do a depth test when rendering the fullscreen quad
    glDisable(GL_DEPTH_TEST);
    shader.use();

    // bind the texture that corresponds to the slide number
    auto texId = textureIds[slideNumber];
    // shader.setInt("screenTexture", 0);
    glBindTexture(GL_TEXTURE_2D, texId);

    // draw the quad
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}
