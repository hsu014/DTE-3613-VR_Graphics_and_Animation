#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "structs.h"
#include "Utils.h"

class Shape {
private:
    	
public:
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

    virtual void fillBuffers() = 0;
    virtual void draw(GLuint shaderProgram);

    GLuint VAO;
    GLuint VBO[4];
    // 0 - position
    // 1 - color
    // 2 - UV
    // 3 - normal
    GLuint EBO;

    GLsizei mIndexCount;
    GLuint mTexture;

    // Material
    glm::vec4 mAmbient = glm::vec4(1.0f);
    glm::vec4 mDiffuse = glm::vec4(1.0f);
    glm::vec4 mSpecular = glm::vec4(1.0f);
    float mShininess = 1.0f;

    glm::mat4 mModelMatrix = glm::mat4(1.0f);
};

class Skybox : public Shape {
    private:

    public:
        Skybox(/*GLuint texture*/);
        void fillBuffers() override;
        void draw(GLuint shaderProgram);
};

class Box : public Shape {
private:
    float mSizeX;
    float mSizeY;
    float mSizeZ;
    
public:
    Box(float size_x, float size_y, float size_z);
	void fillBuffers() override;
    //void draw() override;
};

class Pyramid : public Shape {
private:
    float mSizeX;
    float mHeight;
    float mSizeZ;

public:
    Pyramid(float size_x, float height, float size_z);
    void fillBuffers() override;
    //void draw() override;
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
    int mSectors;
    int mStacks;

public:
    Sphere(int sectors, int stacks);
    void fillBuffers() override;
    //void draw() override;
};