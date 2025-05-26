#version 430 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D u_ssaoInput;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_ssaoInput, 0));
    float result = 0.0;

    // Simple box blur
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_ssaoInput, texCoords + offset).r;
        }
    }

    result /= 9.0;

    FragColor = vec4(result, result, result, 1.0);
}  