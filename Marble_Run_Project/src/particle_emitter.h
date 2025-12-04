#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>

#include "Utils.h"


struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 color;
	float life;
	float size;

	Particle()
		: position(0.0f), velocity(0.0f), color(1.0f), life(0), size(1.0f) { }
};


class Emitter {
public:
	Emitter(int particlesPerSecond=0, float particleLifetime=0, float radius=0, float particleSize=0.1, GLuint texture=0);
	~Emitter();

	virtual void initializeParticles();
	void fillBuffers();
	void setPosition(glm::vec3 position);
	void resetParticle(Particle& p);
	int findUnusedParticle();
	void setPBody(btRigidBody* pBody);

	virtual void updateParticles(float dt) = 0;
	void renderParticles(GLuint shaderProgram);

	/// Variables
	GLuint VAO;
	GLuint VBO[2];
	// 0 - position
	// 1 - UV
	GLuint EBO;

	std::vector<Particle> mParticlesContainer;
	int mLastUsedParticle = 0;
	int mParticlesPerSecond;
	float mTimeBetweenParticles;
	float mTimeSinceLast = 0.0;

	float mParticleLifetime;
	float mRadius;
	float mSize;

	GLuint mTexture;
	glm::vec3 mPosition = glm::vec3(0.0f);
	btRigidBody* m_pBody = nullptr;
};

class FlameEmitter : public Emitter {
public:
	using Emitter::Emitter;
	void updateParticles(float dt) override;
};

class SmokeEmitter : public Emitter {
public:
	using Emitter::Emitter;
	void updateParticles(float dt) override;
};

class TrailEmitter : public Emitter {	
public:
	TrailEmitter(float timeBetween = 0.1, float particleLifetime = 0, float particleSize = 0.1, GLuint texture = 0);
	void initializeParticles() override;
	void updateParticles(float dt) override;

	glm::vec4 mColor;
};


