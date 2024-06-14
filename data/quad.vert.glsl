#version 330 core
// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

// Fullscreen quad shader
// Source: https://learnopengl.com/Advanced-OpenGL/Framebuffers

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
