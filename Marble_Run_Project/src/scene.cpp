#include "scene.h"



Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::update(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 cameraPos)
{
	mViewMatrix = viewMatrix;
	mProjectionMatrix = projectionMatrix;
	mCameraUp = glm::vec3(mViewMatrix[0][1], mViewMatrix[1][1], mViewMatrix[2][1]);
	mCameraFront = glm::vec3(mViewMatrix[0][2], mViewMatrix[1][2], mViewMatrix[2][2]);
	mCameraPos = cameraPos;
}

void Scene::setShaders(GLuint basicShader, GLuint phongShader, GLuint skyboxShader)
{
	mBasicShader = basicShader;
	mPhongShader = phongShader;
	mSkyboxShader = skyboxShader;
}

void Scene::setParticleShaders(GLuint particleShader)
{
	mParticleShader = particleShader;
}

void Scene::setAmbientLight(glm::vec4 color)
{
	mLights.ambient.color = color;
}

void Scene::addDirectionLight(DirectionalLight light)
{
	mLights.directional.push_back(light);
}

void Scene::addPointLight(PointLight light)
{
	mLights.point.push_back(light);
}

void Scene::addSkybox(Skybox* skybox)
{
	mSkybox.push_back(skybox);
}

void Scene::addBaseShape(Shape* shape)
{
	mBasicShapes.push_back(shape);
}

void Scene::addPhongShape(Shape* shape)
{
	mPhongShapes.push_back(shape);
}

void Scene::prepareShaderSkybox()
{
	glm::mat4 view = glm::mat4(glm::mat3(mViewMatrix)); // remove translation from the view matrix

	GLuint shaderProgram = mSkyboxShader;
	glUseProgram(shaderProgram);
	shaderSetMat4(shaderProgram, "uView", view);
	shaderSetMat4(shaderProgram, "uProjection", mProjectionMatrix);
}

void Scene::prepareShaderBasic()
{
	GLuint shaderProgram = mBasicShader;
	glUseProgram(shaderProgram);
	shaderSetMat4(shaderProgram, "uView", mViewMatrix);
	shaderSetMat4(shaderProgram, "uProjection", mProjectionMatrix);
}

void Scene::prepareShaderPhong()
{
	GLuint shaderProgram = mPhongShader;
	glUseProgram(shaderProgram);
	shaderSetMat4(shaderProgram, "uView", mViewMatrix);
	shaderSetMat4(shaderProgram, "uProjection", mProjectionMatrix);
	shaderSetVec3(shaderProgram, "uViewPos", mCameraPos);
	shaderSetInt(shaderProgram, "numPointLights", mLights.point.size());

	// Ambient
	shaderSetVec4(shaderProgram, "ambientLight", mLights.ambient.color);

	// Directional
	DirectionalLight dirLight = mLights.directional[0];
	shaderSetVec3(shaderProgram, "dirLight.direction", dirLight.direction);
	shaderSetVec4(shaderProgram, "dirLight.ambient", dirLight.ambient);
	shaderSetVec4(shaderProgram, "dirLight.diffuse", dirLight.diffuse);
	shaderSetVec4(shaderProgram, "dirLight.specular", dirLight.specular);

	// Point
	for (int i = 0; i < mLights.point.size(); i++) {
		PointLight pointLight = mLights.point[i];

		shaderSetVec3(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].position").c_str(), pointLight.position);
		shaderSetVec4(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].ambient").c_str(), pointLight.ambient);
		shaderSetVec4(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].diffuse").c_str(), pointLight.diffuse);
		shaderSetVec4(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].specular").c_str(), pointLight.specular);
		shaderSetFloat(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].constant").c_str(), pointLight.constant);
		shaderSetFloat(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].linear").c_str(), pointLight.linear);
		shaderSetFloat(shaderProgram, (std::string("pointLight[") + std::to_string(i) + "].quadratic").c_str(), pointLight.quadratic);
	}
}

void Scene::prepareShaderParticle()
{
	GLuint shaderProgram = mParticleShader;
	glUseProgram(shaderProgram);
	shaderSetMat4(shaderProgram, "uView", mViewMatrix);
	shaderSetMat4(shaderProgram, "uProjection", mProjectionMatrix);
	shaderSetVec3(shaderProgram, "cameraUp", mCameraUp);
	shaderSetVec3(shaderProgram, "cameraFront", mCameraFront);
}

void Scene::drawSkybox()
{
	if (mSkybox.size() >= 1 ) {
		prepareShaderSkybox();
		mSkybox[0]->draw(mSkyboxShader);

	}
	else {
		std::cout << "Skybox doesn't exist\n";
	}
}

void Scene::drawBaseShapes()
{
	prepareShaderBasic();
	for (Shape* shape : mBasicShapes) {
		shape->draw(mBasicShader);
	}
}

void Scene::drawPhongShapes()
{
	prepareShaderPhong();
	for (Shape* shape : mPhongShapes) {
		shape->draw(mPhongShader);
	}
}

void Scene::draw()
{
	drawSkybox();
	drawBaseShapes();
	drawPhongShapes();
}