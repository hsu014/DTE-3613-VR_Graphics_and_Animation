#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

out vec3 fragPos;
out vec3 color;
out vec2 texCoord;
out vec3 normal;
out vec4 fragPosLightSpace;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uNormal;
uniform mat4 uLightSpaceMatrix;

void main() 
{
	fragPos = vec3(uModel * vec4(inPosition, 1.0));
	normal = mat3(uNormal) * inNormal;
	color = inColor;
    texCoord = inTexCoord;
	fragPosLightSpace = uLightSpaceMatrix * vec4(fragPos, 1.0);

    gl_Position = uProjection * uView * uModel * vec4(inPosition, 1.0);
}