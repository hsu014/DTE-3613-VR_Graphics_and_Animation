#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

#include "structs.h"
#include "Utils.h"

//const float PI = acos(-1.0f);

class Shape {  	
public:
    const float PI = acos(-1.0f);
    ~Shape();
    
    void initBuffers();
    void fillVertexBuffer(std::vector<float> vertices);
    void fillColorBuffer(std::vector<float> colors);
    void fillUVBuffer(std::vector<float> textureUVs);
    void fillNormalBuffer(std::vector<float> normals);
    void fillIndexBuffer(std::vector<unsigned int> indices);

    void setModelMatrix(glm::mat4 modelMatrix);
    void useTexture(GLuint texture);
    void setMaterial(MaterialType mat);
    void setPBody(btRigidBody* pBody);
    void castShadow(bool castShadow = true);

    virtual void fillBuffers() = 0;
    virtual void draw(GLuint shaderProgram);

    /// Variables
    GLuint VAO;
    GLuint VBO[4];
    // 0 - position
    // 1 - color
    // 2 - UV
    // 3 - normal
    GLuint EBO;

    GLsizei mIndexCount;
    GLuint mTexture;
    bool mCastShadow = true;
    bool mRecieveShadow = true; // unused?
    std::vector<float> mVertices;
    std::vector<unsigned int> mIndices;

    // Material
    glm::vec4 mAmbient = glm::vec4(1.0f);
    glm::vec4 mDiffuse = glm::vec4(1.0f);
    glm::vec4 mSpecular = glm::vec4(1.0f);
    float mShininess = 1.0f;

    glm::mat4 mModelMatrix = glm::mat4(1.0f);

    // Bullet
    btRigidBody* m_pBody = nullptr;
};

class Skybox : public Shape {
public:
    Skybox(GLuint texture);
    void fillBuffers() override;
    void draw(GLuint shaderProgram) override;
};

class Box : public Shape {
private:
    float mSizeX;
    float mSizeY;
    float mSizeZ;
    
public:
    Box(float size_x = 0.5f, float size_y = 0.5f, float size_z = 0.5f);
	void fillBuffers() override;
};

class Pyramid : public Shape {
private:
    float mSizeX;
    float mHeight;
    float mSizeZ;

public:
    Pyramid(float size_x = 0.5f, float height = 1.0f, float size_z = 0.5f);
    void fillBuffers() override;
};

class Plane : public Shape {
private:
    float mSizeX;
    float mSizeZ;
public:
    Plane(float size_x, float size_z);
    void fillBuffers() override;
};

class CompositePlane : public Shape {
private:
    int mWidth;
    int mDepth;
    std::shared_ptr<std::vector<std::vector<float>>> mHeightMap;

public:
    CompositePlane(int width, int depth, GLuint texture);
    CompositePlane(GLuint texture, 
        std::shared_ptr<std::vector<std::vector<float>>> heightMap);
    void fillBuffers() override;
    //void draw() override;
};

class Sphere : public Shape {
private:
    float mRadius;
    int mSectors;
    int mStacks;

public:
    Sphere(float radius = 1.0f, int sectors = 50, int stacks = 50);
    void fillBuffers() override;
};

class Cylinder : public Shape {
private:
    float mRadius;
    float mHeight;
    int mSectors;
public:
    Cylinder(float radius = 0.5f, float height = 1.0f, int sectors = 50);
    void fillBuffers() override;
};

class HalfPipe : public Shape {
private:
    float mInnerRadius;
    float mOuterRadius;
    float mLength;
    int mSectors;
public:
    HalfPipe(float innerRadius = 0.9f, float outerRadius = 1.0f, float length = 1.0f, int sectors = 10);
    void fillBuffers() override;
};

class HalfPipeTrack : public Shape {
private:
    std::vector<TrackSupport> mSupports;
    int mSectors;
public:
    HalfPipeTrack(std::vector<TrackSupport> supports, int sectors = 10);
    void fillBuffers() override;
};