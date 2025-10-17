#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <vector>

#include "shape.h"
#include "particle_emitter.h"


struct Camera {
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float yaw;
    float pitch;
};

struct Time {
    double prev;
    double current;
    double dt;
};

struct ShaderProgram {
    GLuint base;
    GLuint red;
    GLuint pair;
    GLuint texture;
    GLuint phong;
    GLuint particle;
};

struct AmbientLight {
    glm::vec4 color;
};

struct DirectionalLight {
    glm::vec3 direction;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

struct PointLight {
    glm::vec3 position;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Light {
    AmbientLight ambient;
    std::vector<DirectionalLight> directional;
    std::vector<PointLight> point;
};

struct MaterialType {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
};

struct RenderInfo {
    Camera camera;
    Light light;
    Time time;
    ShaderProgram shaderProgram;
    glm::mat4 rotationMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    std::map<std::string, Shape*> shape;
    std::map<std::string, MaterialType> material;
    std::map<std::string, GLuint> texture;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    Emitter emitter;
};