#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <vector>

#include "structs.h"
#include "shape.h"
#include "particle_emitter.h"
#include "camera.h"

struct RenderInfo {
    Time time;

    std::map<std::string, GLuint> texture;
    std::map<std::string, GLuint> skyboxTexture;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    
    Bullet bullet;

    Camera* camera = nullptr;
    //Scene scene;
    // Move to scene:
    Emitter emitter;
};