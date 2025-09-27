#version 330 core

out vec4 fragColor;

in vec3 FragPos;
in vec3 Color;
in vec2 TexCoord;
in vec3 Normal;

struct DirectionalLight
{	
	vec3 direction;

	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;
};

struct PointLight
{	
	vec3 position;

	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	
	float constant;
    float linear;
    float quadratic;
};

struct Material
{	
	vec4 ambient;  
	vec4 diffuse;  
	vec4 specular;  
	float shininess;
};

uniform sampler2D ourTexture;

uniform vec3 viewPos;
uniform vec4 ambientLight;
uniform DirectionalLight dirLight;
uniform PointLight pointLight;
uniform Material material;

// function prototypes
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
	// Properties
	vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

	// phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

	// phase 2: point lights
	result += CalcPointLight(pointLight, norm, FragPos, viewDir);

	fragColor = vec4(result, 1.0); // * vec4(Color, 1.0);

}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    // vec3 ambient = vec3(light.ambient) * vec3(material.ambient);
	// vec3 diffuse = vec3(light.diffuse) * diff * vec3(material.diffuse);
	// vec3 specular = vec3(light.specular) * spec * vec3(material.specular);
	vec3 ambient = vec3(light.ambient) * Color;
	vec3 diffuse = vec3(light.diffuse) * diff * Color;
    vec3 specular = vec3(light.specular) * spec * Color;
    
	return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    // vec3 ambient = vec3(light.ambient) * vec3(material.diffuse);
    // vec3 diffuse = vec3(light.diffuse) * diff * vec3(material.diffuse);
	// vec3 specular = vec3(light.specular) * spec * vec3(material.specular);
	vec3 ambient = vec3(light.ambient) * Color;
    vec3 diffuse = vec3(light.diffuse) * diff * Color;
    vec3 specular = vec3(light.specular) * spec * Color;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}