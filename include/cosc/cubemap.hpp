#pragma once
#include "cosc/shader.hpp"
#include "cosc/util.hpp"

namespace cosc {

/// An OpenGL cubemap
class Cubemap {
public:
    /// Loads a cubemap from a directory of PNG images in the form front.png, back.png, etc.
    /// @param dataDir path to the data directory: to load the cubemap shader
    /// @param cubeMapDir path to the cube map PNG image directory
    explicit Cubemap(const fs::path &dataDir, const fs::path &cubeMapDir);

    /// Draws using the internal managed shader.
    void draw();

private:
    Shader shader;
    unsigned int textureId;
};

}
