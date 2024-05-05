#pragma once
#include "cosc/camera.hpp"
#include "cosc/shader.hpp"

namespace cosc {

/// An OpenGL cubemap. Based on: https://learnopengl.com/Advanced-OpenGL/Cubemaps
class Cubemap {
public:
    /// Loads a cubemap from a directory of PNG images in the form front.png, back.png, etc.
    /// @param dataDir path to the data directory: to load the cubemap shader
    /// @param cubeMapDir path to the cube map PNG image directory
    explicit Cubemap(const fs::path &dataDir, const fs::path &cubeMapDir);

    /// Draws using the internal managed shader.
    /// MUST BE CALLED AT THE END OF THE SCENE!
    void draw(const Camera &camera);

private:
    Shader shader;
    unsigned int textureId;
    unsigned int vbo;
    unsigned int vao;
};

} // namespace cosc
