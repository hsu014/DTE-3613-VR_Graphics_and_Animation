#version 330 core

layout (location = 0) in vec3 inOffset;
layout (location = 1) in vec2 inTexCoord;

//uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uCameraUp;
uniform vec3 uCameraFront;

// Set by each particles draw function
uniform vec4 inColor;
uniform vec3 inPosition;
uniform float inSize;

out vec4 ourColor;
out vec2 texCoord;

void main() {
    // test:
    //gl_Position = uProjection * uView * vec4(inOffset, 1.0);


    vec3 pos = inPosition;
    vec3 offset = inOffset * inSize;

    vec3 right = normalize(cross(uCameraUp, uCameraFront));

    // No billboarding
    //pos += offset;

    // Spherical billboarding
    pos = pos + (right * offset.x) + (uCameraUp * offset.y);

    gl_Position = uProjection * uView * vec4(pos, 1.0);
    //gl_Position = uProjection * uView * uModel * vec4(pos, 1.0);
    ourColor = inColor;
    texCoord = inTexCoord;
}