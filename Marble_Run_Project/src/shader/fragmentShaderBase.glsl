#version 330 core

out vec4 fragColor;

in vec3 ourColor;
in vec2 texCoord;

uniform sampler2D ourTexture;
uniform int useTexture; 

void main() {
    if (useTexture == 1) {
        fragColor = texture(ourTexture, texCoord);
    }
    else {
        fragColor = vec4(ourColor, 1.0f);
    }
}