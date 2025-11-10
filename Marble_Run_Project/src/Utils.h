#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
//#include <glm/gtx/euler_angles.hpp>


class Utils
{
private:
	static std::string readShaderFile(const char *filePath);
	static void printShaderLog(GLuint shader);
	static void printProgramLog(int prog);
	static GLuint prepareShader(int shaderTYPE, const char *shaderPath);
	static int finalizeShaderProgram(GLuint sprogram);

public:
	Utils();
	static bool checkOpenGLError();
	static GLuint createShaderProgram(const char *vp, const char *fp);
	static GLuint loadTexture(const char *texImagePath);
	static GLuint loadCubeMap(const char *mapDir);
	static std::vector<std::vector<float>> loadHeightMap(const char* texImagePath);

};

void shaderSetVec3(GLuint shaderProgram, const char* name, glm::vec3& value);
void shaderSetVec4(GLuint shaderProgram, const char* name, glm::vec4& value);
void shaderSetMat4(GLuint shaderProgram, const char* name, glm::mat4& value);
void shaderSetFloat(GLuint shaderProgram, const char* name, float value);
void shaderSetInt(GLuint shaderProgram, const char* name, int value);