#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

out vec3 FragPos;
out vec3 Color;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 uModel;
uniform mat4 uModelView;
uniform mat4 uProjection;
uniform mat4 uNormal;

void main() 
{
	FragPos = vec3(uModel * vec4(inPosition, 1.0));
	Normal = mat3(uNormal) * inNormal;
	Color = inColor;
    TexCoord = inTexCoord;

    gl_Position = uProjection * uModelView * vec4(inPosition, 1.0);
}