#version 330 core
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
    // float amount = 0.06;
    // vec2 rOffset = vec2(amount, amount) * spectralEnergyRatio;
    // vec2 gOffset = vec2(amount, amount) * spectralEnergyRatio;
    // vec2 bOffset = vec2(0.0, 0.0); //vec2(-amount, -amount) * spectralEnergyRatio;
    //
    // vec4 rValue = texture2D(screenTexture, TexCoords - rOffset);
    // vec4 gValue = texture2D(screenTexture, TexCoords - gOffset);
    // vec4 bValue = texture2D(screenTexture, TexCoords - bOffset);
    //
    // FragColor = vec4(rValue.r, gValue.g, bValue.b, 1.0);

    vec4 colour = texture2D(screenTexture, TexCoords);
    float amount = 0.06 * spectralEnergyRatio;
    colour.r = texture2D(screenTexture, vec2(TexCoords.x + amount, TexCoords.y)).r;
    colour.g = texture2D(screenTexture, TexCoords).g;
    colour.b = texture2D(screenTexture, vec2(TexCoords.x - amount, TexCoords.y)).b;

    FragColor = colour;
}

