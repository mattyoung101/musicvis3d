#version 330 core
// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Fullscreen quad shader
// Source: https://learnopengl.com/Advanced-OpenGL/Framebuffers

void main() {
    FragColor = texture(screenTexture, TexCoords);
}
