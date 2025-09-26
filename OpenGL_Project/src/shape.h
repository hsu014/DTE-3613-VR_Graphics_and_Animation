#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

class Shape {
private:
	
public:
	GLuint VAO;
    GLuint VBO[4];
    // 0 - position
    // 1 - color
    // 2 - UV
    // 3 - normal
	GLuint EBO;

    

    virtual void fillBuffers() = 0;
    virtual void draw() = 0;
};

class TestShape : public Shape {
private:

public:
    TestShape();
    ~TestShape();
    void fillBuffers() override;
    void draw() override;
};

class Box : public Shape {
private:

public:
    Box();
    ~Box();
	void fillBuffers() override;
    void draw() override;
};

class Pyramid : public Shape {
private:

public:
    Pyramid();
    ~Pyramid();
    void fillBuffers() override;
    void draw() override;
};

class CompositePlane : public Shape {
private:
    int numVBOs = 4;
    int mWidth;
    int mDepth;
    GLsizei indexCount;
    
    std::shared_ptr<std::vector<std::vector<float>>> mHeightMap;

public:
    GLuint mTexture;
    
    CompositePlane(int width, int depth, GLuint texture);
    CompositePlane(GLuint texture, 
        std::shared_ptr<std::vector<std::vector<float>>> heightMap);
    ~CompositePlane();
    void fillBuffers() override;
    void draw() override;

};

class Sphere : public Shape {
private:
    int numVBOs = 4;
    int mSectors;
    int mStacks;
    GLsizei indexCount;

public:
    GLuint mTexture;

    Sphere(int sectors, int stacks);
    ~Sphere();
    void fillBuffers() override;
    void draw() override;

};