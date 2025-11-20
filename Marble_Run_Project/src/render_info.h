#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <vector>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "structs.h"
#include "shape.h"
#include "particle_emitter.h"
#include "camera.h"

struct RenderInfo {
    Time time;
    Bullet bullet;
    Camera* camera = nullptr;

    std::map<std::string, GLuint> texture;
    std::map<std::string, GLuint> skyboxTexture;
    std::map<std::string, std::shared_ptr<std::vector<std::vector<float>>>> heightMap;
    std::vector<SphereInfo> sphereinfo;
    btGhostObject* finishLine = nullptr;
};