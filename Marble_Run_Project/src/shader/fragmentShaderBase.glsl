#version 330 core

out vec4 fragColor;

in vec2 texCoord;

struct Material
{	
	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	float shininess;
};

uniform sampler2D ourTexture;
uniform int useTexture; 
uniform Material material;

void main() {
    if (useTexture == 1) {
        fragColor = texture(ourTexture, texCoord);
    }
    else {
        fragColor = vec4(vec3(material.ambient), 1.0f);
    }
}