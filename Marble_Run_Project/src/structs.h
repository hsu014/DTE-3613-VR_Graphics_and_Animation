#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>


struct Time {
    double prev;
    double current;
    double dt;
};

struct TrackSupport {
    float x, y, z;
    // Degrees of rotation around y-axis. Starting along x-axis?
    float angle; 
    float innerRadius;
    float outerRadius;
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

struct Bullet {
    btCollisionConfiguration* pCollisionConfiguration;
    btCollisionDispatcher* pDispatcher;
    btBroadphaseInterface* pBroadphase;
    btConstraintSolver* pSolver;
    btDynamicsWorld* pWorld;
};

struct MaterialType {
    glm::vec4 ambient = glm::vec4(1.0f);
    glm::vec4 diffuse = glm::vec4(1.0f);
    glm::vec4 specular = glm::vec4(1.0f);
    float shininess = 10.0f;
};

//struct MaterialType {
//    glm::vec4 ambient;
//    glm::vec4 diffuse;
//    glm::vec4 specular;
//    float shininess;
//};

struct Material {
    MaterialType brass = { 
        {0.329412f, 0.223529f, 0.027451f, 1.0f},
        {0.780392f, 0.568627f, 0.113725f, 1.0f},
        {0.992157f, 0.941176f, 0.807843f, 1.0f},
        27.8974f
    };
    MaterialType bronze = {
        {0.2125f, 0.1275f, 0.054f, 1.0f},
        {0.714f, 0.4284f, 0.18144f, 1.0f},
        {0.393548f, 0.271906f, 0.166721f, 1.0f},
        25.6f
    };
    MaterialType polished_bronze = {
        {0.25f, 0.148f, 0.06475f, 1.0f},
        {0.4f, 0.2368f, 0.1036f, 1.0f},
        {0.774597f, 0.458561f, 0.200621f, 1.0f},
        76.8f
    };
    MaterialType chrome = {
        {0.25f, 0.25f, 0.25f, 1.0f},
        {0.4f, 0.4f, 0.4f, 1.0f},
        {0.774597f, 0.774597f, 0.774597f, 1.0f},
        76.8f
    };
    MaterialType copper = {
        {0.19125f, 0.0735f, 0.0225f, 1.0f},
        {0.7038f, 0.27048f, 0.0828f, 1.0f},
        {0.256777f, 0.137622f, 0.086014f, 1.0f},
        12.8f
    };
    MaterialType polished_copper = {
        {0.2295f, 0.08825f, 0.0275f, 1.0f },
        {0.5508f, 0.2118f, 0.066f, 1.0f },
        {0.580594f, 0.223257f, 0.0695701f, 1.0f },
        51.2f
    };
    MaterialType tin = {
        {0.105882f, 0.058824f, 0.113725f, 1.0f },
        {0.427451f, 0.470588f, 0.541176f, 1.0f },
        {0.333333f, 0.333333f, 0.521569f, 1.0f },
        9.84615f
    };
    MaterialType gold = {
        {0.24725f, 0.1995f, 0.0745f, 1.0f },
        {0.75164f, 0.60648f, 0.22648f, 1.0f },
        {0.628281f, 0.555802f, 0.366065f, 1.0f },
        51.2f
    };
    MaterialType polished_gold = {
        {0.24725f, 0.2245f, 0.0645f, 1.0f },
        {0.34615f, 0.3143f, 0.0903f, 1.0f },
        {0.797357f, 0.723991f, 0.208006f, 1.0f},
        83.2f
    };
    MaterialType silver = {
        {0.19225f, 0.19225f, 0.19225f, 1.0f },
        {0.50754f, 0.50754f, 0.50754f, 1.0f},
        {0.508273f, 0.508273f, 0.508273f, 1.0f },
        51.2f
    }; 
    MaterialType polished_silver = {
        {0.23125f, 0.23125f, 0.23125f, 1.0f },
        {0.2775f, 0.2775f, 0.2775f, 1.0f },
        {0.773911f, 0.773911f, 0.773911f, 1.0f },
        89.6f
    };
    MaterialType emerald = {
        {0.0215f, 0.1745f, 0.0215f, 0.55f },
        {0.07568f, 0.61424f, 0.07568f, 0.55f },
        {0.633f, 0.727811f, 0.633f, 0.55f },
        76.8f
    };
    MaterialType ruby = {
        {0.1745f, 0.01175f, 0.01175f, 0.55f },
        {0.61424f, 0.04136f, 0.04136f, 0.55f },
        {0.727811f, 0.626959f, 0.626959f, 0.55f },
        76.8f
    };
    MaterialType obsidian = {
        {0.05375f, 0.05f, 0.06625f, 0.82f },
        {0.18275f, 0.17f, 0.22525f, 0.82f},
        {0.332741f, 0.328634f, 0.346435f, 0.82f },
        38.4f
    };
    MaterialType perl = {
        {0.25f, 0.20725f, 0.20725f, 0.922f },
        {1.0f, 0.829f, 0.829f, 0.922f },
        {0.296648f, 0.296648f, 0.296648f, 0.922f },
        11.264f
    };
};

struct SphereInfo {
    MaterialType material;
    GLuint texture;
    float radius = 0.1f;
    float mass = 1.0f;
    float restitution = 0.6f;
    float friction = 0.8f;
    bool player = false;
};
