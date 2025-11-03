#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>


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
    GLuint phong;
    GLuint particle;
    GLuint skybox;
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

struct Bullet {
    btCollisionConfiguration* pCollisionConfiguration;
    btCollisionDispatcher* pDispatcher;
    btBroadphaseInterface* pBroadphase;
    btConstraintSolver* pSolver;
    btDynamicsWorld* pWorld;
};