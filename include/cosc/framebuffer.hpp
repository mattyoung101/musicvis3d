#pragma once
#include "cosc/shader.hpp"
namespace cosc {

class FrameBuffer {
public:
    /// Creates a new framebuffer with the specified post processing shader. The quad shader is loaded
    /// automatically.
    /// Must also provide viewport width and height.
    explicit FrameBuffer(const fs::path &dataDir, const std::string &postShader, int width, int height);

    /// Binds the framebuffer. Draw calls will go then to the framebuffer.
    void bind();

    /// Draws the framebuffer to the screen
    void draw(float spectralEnergyRatio);

private:
    unsigned int vbo;
    unsigned int vao;

    /// Framebuffer object
    unsigned int frameBuffer;
    /// Colour texture which we can sample from (R/W)
    unsigned int textureColour;
    /// Render buffer which we can't sample, for depth and stencil (R/O)
    unsigned int renderBuffer;

    bool bound = false;
    cosc::Shader quadShader;
};

} // namespace cosc
