#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

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

    virtual void fillBuffers() = 0;
    virtual void draw() = 0;

    GLuint VAO;
    GLuint VBO[4];
    // 0 - position
    // 1 - color
    // 2 - UV
    // 3 - normal
    GLuint EBO;
    GLsizei mIndexCount;
};

class Box : public Shape {
private:
    float mSizeX;
    float mSizeY;
    float mSizeZ;
    
public:
    Box(float size_x, float size_y, float size_z);
	void fillBuffers() override;
    void draw() override;
};

class Pyramid : public Shape {
private:
    float mSizeX;
    float mHeight;
    float mSizeZ;

public:
    Pyramid(float size_x, float height, float size_z);
    void fillBuffers() override;
    void draw() override;
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
    void draw() override;

    GLuint mTexture;
};

class Sphere : public Shape {
private:
    int mSectors;
    int mStacks;
    GLsizei indexCount;

public:
    GLuint mTexture;

    Sphere(int sectors, int stacks);
    Sphere(int sectors, int stacks, GLuint texture);
    void fillBuffers() override;
    void draw() override;

};