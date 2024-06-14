#version 330 core
// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

// Source: https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.1.cubemaps_skybox/6.1.skybox.vs

void main() {
    FragColor = texture(skybox, TexCoords);
}
