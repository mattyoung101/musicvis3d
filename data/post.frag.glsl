#version 330 core
// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float spectralEnergyRatio;

// Post processing fragment shader
// Based on:
// - https://learnopengl.com/Advanced-OpenGL/Framebuffers
// - https://gamedev.stackexchange.com/questions/58408/how-would-you-implement-chromatic-aberration
// - https://github.com/mattyoung101/hackathon23/blob/master/shaders/chromatic.gdshader
// - https://godotshaders.com/shader/chromatic-abberation-with-offset/

void main() {
    vec4 colour;
    float amount = 0.06 * spectralEnergyRatio;
    colour.r = texture2D(screenTexture, vec2(TexCoords.x + amount, TexCoords.y)).r;
    colour.g = texture2D(screenTexture, TexCoords).g;
    colour.b = texture2D(screenTexture, vec2(TexCoords.x - amount, TexCoords.y)).b;
    colour.a = 1.0;

    FragColor = colour;
}

