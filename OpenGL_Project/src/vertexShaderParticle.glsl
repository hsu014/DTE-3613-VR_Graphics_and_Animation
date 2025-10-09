#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 uModelView;
uniform mat4 uProjection;

out vec2 texCoord;

void main() {
    gl_Position = uProjection * uModelView * vec4(aPos, 1.0);
    texCoord = aTex;
}