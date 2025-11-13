#version 330 core

out vec4 fragColor;

in vec3 fragPos;
in vec2 texCoord;
in vec3 normal;
in vec4 fragPosLightSpace;

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
uniform sampler2D shadowMap;
uniform int useTexture;
uniform int numPointLights;

uniform vec3 uViewPos;
uniform vec4 ambientLight;
uniform DirectionalLight dirLight;
uniform PointLight pointLight[10];
uniform Material material;



// function prototypes
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec3 normal, vec3 lightDir);


void main() {
	// Properties
	vec3 norm = normalize(normal);
    vec3 viewDir = normalize(uViewPos - fragPos);

	// Directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

	// Point lights
    for(int i = 0; i < numPointLights; i++) {
	    result += CalcPointLight(pointLight[i], norm, fragPos, viewDir);
        }

    // ambient light
    result += vec3(ambientLight) * vec3(material.ambient);

    if (useTexture == 1) {
        fragColor = texture(ourTexture, texCoord) * vec4(result, 1.0f);
    }
    else {
        fragColor = vec4(result, 1.0f);
    }
}



vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // combine results
    vec3 ambient = vec3(light.ambient) * vec3(material.ambient);
	vec3 diffuse = vec3(light.diffuse) * diff * vec3(material.diffuse);
	vec3 specular = vec3(light.specular) * spec * vec3(material.specular);
    
    // shadow factor
    float shadow = ShadowCalculation(normal, lightDir);

    return ambient + (1.0 - shadow) * (diffuse + specular);
}



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
    vec3 ambient = vec3(light.ambient) * vec3(material.diffuse);
    vec3 diffuse = vec3(light.diffuse) * diff * vec3(material.diffuse);
	vec3 specular = vec3(light.specular) * spec * vec3(material.specular);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}



float ShadowCalculation(vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;  // [-1, 1]
    projCoords = projCoords * 0.5 + 0.5;                            // [ 0, 1]

    // Check if outside light frustum
    if (projCoords.z > 1.0) {
        return 0.0;
    }
    if (projCoords.z < 0.0) {
        return 0.0;
    }

    // Get closest depth from light's POV
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Bias to reduce shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    // --- PCF (3x3 soft shadows) ---
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}