#include "scene.h"



Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::update(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	mViewMatrix = viewMatrix;
	mProjectionMatrix = projectionMatrix;
	mCameraUp = glm::vec3(mViewMatrix[0][1], mViewMatrix[1][1], mViewMatrix[2][1]);
	mCameraFront = glm::vec3(mViewMatrix[0][2], mViewMatrix[1][2], mViewMatrix[2][2]);

}

void Scene::setShaders(GLuint basicShader, GLuint phongShader, GLuint skyboxShader)
{
	mBasicShader = basicShader;
	mPhongShader = phongShader;
	mSkyboxShader = skyboxShader;
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
}

void Scene::prepareShaderParticle()
{
}

void Scene::drawSkybox()
{
	if (mSkybox.size() >= 1 ) {
		// std::cout << "Skybox exists\n";
		prepareShaderSkybox();
		mSkybox[0]->draw(mSkyboxShader);

	}
	else {
		std::cout << "Skybox doesn't exists\n";
	}
}

void Scene::drawBaseShapes()
{
	prepareShaderBasic();
	for (Shape* shape : mBasicShapes) {
		shape->draw(mBasicShader);
	}
}

void Scene::draw()
{
	drawSkybox();
	drawBaseShapes();
}