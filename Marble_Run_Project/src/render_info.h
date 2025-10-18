#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <vector>

#include "structs.h"
#include "shape.h"
#include "particle_emitter.h"

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
    std::map<std::string, GLuint> skybox;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    Emitter emitter;
    Bullet bullet;
};