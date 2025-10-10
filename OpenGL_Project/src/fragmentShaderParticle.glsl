#version 330 core

in vec4 ourColor;
in vec2 texCoord;

uniform sampler2D ourTexture;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(ourTexture, texCoord);
    fragColor = texColor * ourColor;
    //fragColor = ourColor;
}