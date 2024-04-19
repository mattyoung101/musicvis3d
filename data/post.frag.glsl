#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// Fullscreen quad shader
// Source: https://learnopengl.com/Advanced-OpenGL/Framebuffers

void main() {
    FragColor = texture(screenTexture, TexCoords);
}

