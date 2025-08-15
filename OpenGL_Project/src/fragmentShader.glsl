#version 330 core

out vec4 frag_color;
in vec3 ourColor;

void main() {
    frag_color = vec4(ourColor, 1.0f);
}