#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
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


class Emitter
{
private:

public:
	std::vector<Particle> mParticlesContainer;
	int mLastUsedParticle = 0;
	int mNumParticles;
	int mNumNewParticles;
	double mParticleLifetime;
	float mRadius;
	GLuint mTexture;

	GLuint VAO;
	GLuint VBO[2];
	// 0 - position
	// 1 - UV
	GLuint EBO;

	Emitter();
	Emitter(int particlesPerSecond, float particleLifetime, float radius, GLuint texture);
	~Emitter();

	void resetParticle(Particle& p);
	void initializeParticles();
	int findUnusedParticle();
	void updateParticles(double dt);
	void renderParticles(GLuint shaderProgram);
};


