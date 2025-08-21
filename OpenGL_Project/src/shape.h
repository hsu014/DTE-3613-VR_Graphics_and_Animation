#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

class Shape {
private:
	
public:
	GLuint VBO;
	GLuint VAO;
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