#version 330 core

in vec3 FragPos; // fragment vertex position in world space
in vec3 Normal; // vertex normal vector from vertex shader

out vec4 FragColor; // output colour
uniform vec3 viewPos; // camera pos

// fifth-order polynomial approximation of Turbo colour map based on:
// https://observablehq.com/@mbostock/turbo
// Source: https://www.shadertoy.com/view/3t2XzV
vec3 turbo(float x) {
    float r = 0.1357 + x * (4.5974 - x * (42.3277 - x * (130.5887 - x * (150.5666 - x * 58.1375))));
    float g = 0.0914 + x * (2.1856 + x * (4.8052 - x * (14.0195 - x * (4.2109 + x * 2.7747))));
    float b = 0.1067 + x * (12.5925 - x * (60.1097 - x * (109.0745 - x * (88.5066 - x * 26.8183))));
    return vec3(r, g, b);
}

// Inferno colour map
// Source: https://observablehq.com/@flimsyhat/webgl-color-maps

vec3 inferno(float t) {
    const vec3 c0 = vec3(0.0002189403691192265, 0.001651004631001012, -0.01948089843709184);
    const vec3 c1 = vec3(0.1065134194856116, 0.5639564367884091, 3.932712388889277);
    const vec3 c2 = vec3(11.60249308247187, -3.972853965665698, -15.9423941062914);
    const vec3 c3 = vec3(-41.70399613139459, 17.43639888205313, 44.35414519872813);
    const vec3 c4 = vec3(77.162935699427, -33.40235894210092, -81.80730925738993);
    const vec3 c5 = vec3(-71.31942824499214, 32.62606426397723, 73.20951985803202);
    const vec3 c6 = vec3(25.13112622477341, -12.24266895238567, -23.07032500287172);

    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
}

// Based on: https://learnopengl.com/Lighting/Basic-Lighting

void main() {
    // compute angle of this fragment's normal against the camera
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    float angle = max(dot(norm, viewDir), 0.0);

    // ideally we also render this as "patches" as the task sheet requests and similar to Yutong on edstem
    // TODO patches (if required) - needs tex coord

    // colour angle based on inferno colourmap
    FragColor = vec4(inferno(angle), 1.0f);
}
