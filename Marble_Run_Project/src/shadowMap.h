#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include <iostream>
#include <vector>

class ShadowMap {
public:
	ShadowMap();



	const GLuint mSHADOW_WIDTH = 1024;
	const GLuint mSHADOW_HEIGHT = 1024;
	GLuint FBO;
	GLuint mDepthMap;

};

