#include "cosc/cubemap.hpp"
#include "cosc/shader.hpp"
#include "glad/gl.h"
#include <spdlog/spdlog.h>

cosc::Cubemap::Cubemap(const fs::path &dataDir, const fs::path &cubeMapDir)
    : shader(Shader(dataDir / "cubemap.frag.glsl", dataDir / "cubemap.vert.glsl")) {
    SPDLOG_INFO("Instantiating Cubemap");

    // generate and load OpenGL textures
    SPDLOG_INFO("Loading cubemap textures");
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    std::vector<std::string> faces = {"right", "left", "top", "bottom", "front", "back"};
    for (const auto &face : faces) {
        auto facePath = dataDir / cubeMapDir / face / ".png";
        SPDLOG_DEBUG("Loading face: {} from path: {}", face, facePath.string());
    }
}
