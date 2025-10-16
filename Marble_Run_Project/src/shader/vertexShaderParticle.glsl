#version 330 core

layout (location = 0) in vec3 inOffset;
layout (location = 1) in vec2 inTexCoord;

uniform mat4 uModelView;
uniform mat4 uProjection;

uniform vec3 cameraUp;
uniform vec3 cameraFront;
uniform vec4 inColor;
uniform vec3 inPosition;
uniform float inSize;

out vec4 ourColor;
out vec2 texCoord;

void main() {
    vec3 pos = inPosition;
    vec3 offset = inOffset * inSize;

    vec3 right = normalize(cross(cameraUp, cameraFront));

    // No billboarding
    //pos += offset;

    // Spherical billboarding
    pos = pos + (right * offset.x) + (cameraUp * offset.y);

    gl_Position = uProjection * uModelView * vec4(pos, 1.0);
    ourColor = inColor;
    texCoord = inTexCoord;
}