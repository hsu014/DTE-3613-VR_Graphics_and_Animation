#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include "shape.h"
#include "particle_emitter.h"
#include "Utils.h"

class Scene {
private:
	

public:
	Scene();
	~Scene();

	void update(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 cameraPos);

	void setShaders(GLuint basicShader, GLuint phongShader, GLuint skyboxShader);

	//Add Lights
	void setAmbientLight(glm::vec4 color);
	void addDirectionLight(DirectionalLight light);
	void addPointLight(PointLight light);

	void addSkybox(Skybox* skybox);
	void addBaseShape(Shape* shape);
	void addPhongShape(Shape* shape);
	//void addEmitter(Emitter* shape);

	void prepareShaderSkybox();
	void prepareShaderBasic();
	void prepareShaderPhong();
	void prepareShaderParticle();
	
	void drawSkybox();
	void drawBaseShapes();
	void drawPhongShapes();
	//void drawEmitters();

	void draw();

	GLuint mBasicShader;
	GLuint mPhongShader;
	GLuint mSkyboxShader;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;
	glm::vec3 mCameraUp;
	glm::vec3 mCameraFront;
	glm::vec3 mCameraPos;

	Light mLights;
	std::vector<Shape*> mBasicShapes;
	std::vector<Shape*> mPhongShapes;
	std::vector<Skybox*> mSkybox;

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
