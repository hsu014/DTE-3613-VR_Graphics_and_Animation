#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shape.h"
#include "particle_emitter.h"
#include "Utils.h"
#include "camera.h"

class Scene {
public:
	Scene(GLFWwindow* window);

	void initShadowMap();

	void updateLightSpaceMatrix();
	void updateDirLight();
	void update(Camera& camera, double dt);

	void setShaders(GLuint basicShader, GLuint phongShader, GLuint skyboxShader, GLuint shadowMapShader);
	void setParticleShader(GLuint particleShader);

	void setAmbientLight(glm::vec4 color);
	void addDirectionLight(DirectionalLight light);
	void addPointLight(PointLight light, bool visualize = false);

	void addSkybox(Skybox* skybox);
	void addBaseShape(Shape* shape);
	void addPhongShape(Shape* shape);
	void addEmitter(Emitter* emitter);

	void prepareShaderSkybox();
	void prepareShaderBasic();
	void prepareShaderPhong();
	void prepareShaderParticle();
	void prepareShaderShadowMap();
	
	void shadowPass();
	void drawSkybox();
	void drawBaseShapes();
	void drawPhongShapes();
	void drawEmitters();

	void draw();

	// Variables
	GLFWwindow* mWindow;
	double mDt = 0.0;

	GLuint mBasicShader;
	GLuint mPhongShader;
	GLuint mSkyboxShader;
	GLuint mShadowMapShader;
	GLuint mParticleShader;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;
	glm::vec3 mCameraUp;
	glm::vec3 mCameraFront;
	glm::vec3 mCameraPos;

	Light mLights;
	float mLightYaw = 0.0f;
	float mLightPitch = 0.0f;
	std::vector<Shape*> mBasicShapes;
	std::vector<Shape*> mPhongShapes;
	std::vector<Emitter*> mEmitters;
	std::vector<Skybox*> mSkybox;

	// Shadow map
	float mShadowAreaSize = 100;
	// 8192 16384
	const GLuint mSHADOW_WIDTH = 16384;
	const GLuint mSHADOW_HEIGHT = 16384;
	GLuint FBO;
	GLuint mShadowMap;
	glm::mat4 mLightSpaceMatrix;

};

