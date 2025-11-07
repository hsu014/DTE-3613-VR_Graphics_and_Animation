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
private:
	

public:
	Scene(GLFWwindow* window);
	~Scene();

	void initShadowMap();

	void updateLightSpaceMatrix();
	void update(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 cameraPos);
	void update(Camera& camera);

	void setShaders(GLuint basicShader, GLuint phongShader, GLuint skyboxShader, GLuint shadowMapShader);
	void setParticleShaders(GLuint particleShader);

	void setAmbientLight(glm::vec4 color);
	void addDirectionLight(DirectionalLight light);
	void addPointLight(PointLight light, bool visualize = false);

	void addSkybox(Skybox* skybox);
	void addBaseShape(Shape* shape);
	void addPhongShape(Shape* shape);
	//void addEmitter(Emitter* shape);

	void prepareShaderSkybox();
	void prepareShaderBasic();
	void prepareShaderPhong();
	void prepareShaderParticle();
	void prepareShaderShadowMap();
	//void prepareShaderEmitter();
	
	void drawSkybox();
	void drawBaseShapes();
	void drawPhongShapes();
	//void drawEmitters();

	void draw();

	GLFWwindow* mWindow;

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
	std::vector<Shape*> mBasicShapes;
	std::vector<Shape*> mPhongShapes;
	std::vector<Skybox*> mSkybox;

	// Shadow map
	const GLuint mSHADOW_WIDTH = 8192;
	const GLuint mSHADOW_HEIGHT = 8192;
	GLuint FBO;
	GLuint mShadowMap;
	glm::mat4 mLightSpaceMatrix;

};


/*
Scene must know:
  Lists of all objects (shapes)
    Divided into which shader to use
  List of all lights
  ActivateShaderBase
  ActivateShaderPhong(light)
  ActivateShaderParticle

  Update each frame:
  viewMatric
  projectionMatrix
  Camera up *
  Camera front *



Shapes must know
  Model matrix
  Material


Types of shader:
  Base
  Lighting
  Skybox (base?)
  Particle


Needed to render:
Base shader
  Model matrix
  [u]View matrix
  [u]Projection Matrix
  Material
  Texture?

Phong
  Model matrix
  [u]View matrix
  [u]Projection Matrix
  Material
  Texture?
  [u]Lights

Skybox
  [u]View matrix (modify)
  [u]Projection Matrix
  Texture

Emitter
  Model matrix
  [u]View matrix
  [u]Projection Matrix
  [u]Camera up
  [u]Camera front
  Texture


*/
