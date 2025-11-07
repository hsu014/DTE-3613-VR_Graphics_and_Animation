#include "scene.h"

Scene::Scene(GLFWwindow* window) : mWindow(window)
{
	initShadowMap();
}

Scene::~Scene() {}

void Scene::initShadowMap()
{
	glGenFramebuffers(1, &FBO);
	glGenTextures(1, &mShadowMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mShadowMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mSHADOW_WIDTH, mSHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer problem" << std::endl;
	}
	else std::cout << "Framebuffer ok" << std::endl;
}

void Scene::updateLightSpaceMatrix()
{
	glm::vec3 lightBoxPos = {0.0f, 0.0f, 0.0f};
	lightBoxPos = mCameraPos;
	glm::vec3 lightInvDir = glm::normalize(mLights.directional[0].direction * -1.0f);

	if (lightInvDir.y == 1.0f) // Fix edge case
	{
		lightInvDir = glm::normalize(glm::vec3{ 0.0f, 1.0, -0.000000001 });
	}

	float orthoSize = 25.0f;

	glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, orthoSize*2.0f);

	glm::mat4 lightView = glm::lookAt((lightInvDir * orthoSize) + lightBoxPos,	// position
		lightBoxPos,															// target
		glm::vec3(0.0f, 1.0f, 0.0f));											// up

	mLightSpaceMatrix = lightProjection * lightView;
}

void Scene::update(Camera& camera)
{
	mViewMatrix = camera.getViewMatrix();
	mProjectionMatrix = camera.getProjectionMatrix();
	mCameraUp = camera.getCameraUp();
	mCameraFront = camera.getCameraFront();
	mCameraPos = camera.getCameraPos();
	mDt = camera.mDt;

	// if dir light moves:
	updateLightSpaceMatrix();
}


void Scene::setShaders(GLuint basicShader, GLuint phongShader, GLuint skyboxShader, GLuint shadowMapShader)
{
	mBasicShader = basicShader;
	mPhongShader = phongShader;
	mSkyboxShader = skyboxShader;
	mShadowMapShader = shadowMapShader;
}

void Scene::setParticleShader(GLuint particleShader)
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

void Scene::addPointLight(PointLight light, bool visualize)
{
	mLights.point.push_back(light);

	if (visualize) {
		Shape* sphere = new Sphere(0.02, 20, 20);
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, light.position);
		sphere->setModelMatrix(modelMatrix);

		addBaseShape(sphere);
	}
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

void Scene::addEmitter(Emitter* emitter)
{
	mEmitters.push_back(emitter);
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

	// Shadow map
	shaderSetInt(shaderProgram, "ourTexture", 0);
	shaderSetInt(shaderProgram, "shadowMap", 1);
	shaderSetMat4(shaderProgram, "uLightSpaceMatrix", mLightSpaceMatrix);
}

void Scene::prepareShaderParticle()
{
	GLuint shaderProgram = mParticleShader;
	glUseProgram(shaderProgram);
	shaderSetMat4(shaderProgram, "uView", mViewMatrix);
	shaderSetMat4(shaderProgram, "uProjection", mProjectionMatrix);
	shaderSetVec3(shaderProgram, "uCameraUp", mCameraUp);
	shaderSetVec3(shaderProgram, "uCameraFront", mCameraFront);
}

void Scene::prepareShaderShadowMap()
{
	GLuint shaderProgram = mShadowMapShader;
	glUseProgram(shaderProgram);
	shaderSetMat4(shaderProgram, "uLightSpaceMatrix", mLightSpaceMatrix);
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
	// Shadow pass
	prepareShaderShadowMap();
	glViewport(0, 0, mSHADOW_WIDTH, mSHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	for (Shape* shape : mPhongShapes) {
		if (shape->mCastShadow) {
			shape->draw(mShadowMapShader);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Reset viewPort
	int width, height;
	glfwGetWindowSize(mWindow, &width, &height);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render scene
	prepareShaderPhong();
	for (Shape* shape : mPhongShapes) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mShadowMap);
		shape->draw(mPhongShader);
	}
}

void Scene::drawEmitters()
{
	prepareShaderParticle();
	for (Emitter* emitter : mEmitters) {
		emitter->updateParticles(mDt);
		emitter->renderParticles(mParticleShader);
	}
}

void Scene::draw()
{
	glClearColor(0.2f, 0.0f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawPhongShapes();
	drawBaseShapes();
	drawSkybox();
	drawEmitters();
	
}