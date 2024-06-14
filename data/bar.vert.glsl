#version 330 core
// Copyright 2024 Matt Young.
// SPDX-License-Identifier: ISC

layout (location = 0) in vec3 aPos; // vertex position
layout (location = 1) in vec3 aNormal; // vertex normal

out vec3 Normal; // to fragment shader
out vec3 FragPos; // to fragment shader 

uniform mat4 model; // model transform
uniform mat3 modelInv; // inverse model transform
uniform mat4 view; // camera view matrix
uniform mat4 projection; // camera projection matrix

// Sources: 
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/3.model_loading/1.model_loading/1.model_loading.vs
// https://learnopengl.com/Lighting/Basic-Lighting

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = modelInv * aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));
}
