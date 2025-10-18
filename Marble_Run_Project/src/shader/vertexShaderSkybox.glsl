#version 330 core

layout (location = 0) in vec3 inPos;

out vec3 texCoords;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    texCoords = inPos;
    gl_Position = uProjection * uView * vec4(inPos, 1.0);
} 