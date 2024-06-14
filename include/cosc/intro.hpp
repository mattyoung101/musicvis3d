// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC
#pragma once
#include "cosc/shader.hpp"
#include <vector>

namespace cosc {
/// This class handles drawing the intro animation.
class IntroManager {
public:
    explicit IntroManager(const fs::path &dataDir);

    /// Draws the intro slide. Slide number is 0 indexed (0, 1, 2).
    void draw(size_t slideNumber);

private:
    cosc::Shader shader;
    /// Index in this array is the slide number (0, 1, 2) and the value is the GL texture ID.
    std::vector<unsigned int> textureIds;
    unsigned int vbo = 0;
    unsigned int vao = 0;
};
} // namespace cosc
