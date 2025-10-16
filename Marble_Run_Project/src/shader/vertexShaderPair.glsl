#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

out vec3 ourColor;
out vec2 texCoord;
uniform mat4 uModelView;
uniform mat4 uProjection;
uniform float changeColor;

void main() 
{
    float c_color = (changeColor + 1) / 2;

    ourColor = vec3(c_color, 0.2, c_color);
    texCoord = inTexCoord;

    gl_Position = uProjection * uModelView * vec4(inPosition, 1.0);
}