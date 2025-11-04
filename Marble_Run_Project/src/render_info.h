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
    Time time;
    ShaderProgram shaderProgram;

    std::map<std::string, MaterialType> material;
    std::map<std::string, GLuint> texture;
    std::map<std::string, GLuint> skyboxTexture;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    
    Bullet bullet;

    //Scene scene;
    // Move to scene:
    Emitter emitter;
};