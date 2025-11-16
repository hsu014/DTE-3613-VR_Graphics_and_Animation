#include "particle_emitter.h"

Emitter::Emitter(int particlesPerSecond, float particleLifetime, float radius, float particleSize, GLuint texture) :
    mNumNewParticles(particlesPerSecond), mParticleLifetime(particleLifetime),
    mSize(particleSize), mRadius(radius), mTexture(texture)
{
    initializeParticles();
}

Emitter::~Emitter() 
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(2, VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Emitter::initializeParticles()
{
    mNumParticles = mNumNewParticles * mParticleLifetime * 1.5;
    mParticlesContainer.resize(mNumParticles, Particle());

    fillBuffers(); 
}

void Emitter::fillBuffers()
{
    float size = 1.0f;

    float vertices[] = {
         -size, -size, 0.0f,
          size, -size, 0.0f,
          size,  size, 0.0f,
         -size,  size, 0.0f,
    };

    float textureUVs[] = {
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    unsigned int indices[] = {
        0, 2, 1,  // first Triangle
        2, 0, 3,  // second Triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture UV attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureUVs), textureUVs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind VAO
    glBindVertexArray(0);
}

void Emitter::setPosition(glm::vec3 position)
{
    mPosition = position;
}

void Emitter::resetParticle(Particle& p)
{
    p.position = {0.0f, 0.0f, 0.0f };
    p.velocity = {0.0f, 0.0f, 0.0f };
    p.color = {1.0f, 1.0f, 1.0f, 1.0f };
    p.life = 0.0f;
    p.size = mSize;
}

int Emitter::findUnusedParticle()
{
    for (int i = mLastUsedParticle; i < mParticlesContainer.size(); i++) {
        if (mParticlesContainer[i].life <= 0.0) {
            mLastUsedParticle = i;
            return i;
        }
    }

    for (int i = 0; i < mLastUsedParticle; i++) {
        if (mParticlesContainer[i].life <= 0.0) {
            mLastUsedParticle = i;
            return i;
        }
    }

	return 0;
}

void Emitter::setPBody(btRigidBody* pBody)
{
    m_pBody = pBody;

    btTransform trans;
    m_pBody->getMotionState()->getWorldTransform(trans);

    btVector3 pos = trans.getOrigin();
    mPosition = glm::vec3(pos.getX(), pos.getY(), pos.getZ());
}

void Emitter::renderParticles(GLuint shaderProgram)
{
    if (m_pBody) {
        btTransform trans;
        m_pBody->getMotionState()->getWorldTransform(trans);

        btVector3 pos = trans.getOrigin();
        mPosition = glm::vec3(pos.getX(), pos.getY(), pos.getZ());
    }

    // Activate texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindVertexArray(VAO);

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    int active_p = 0;
    for (Particle& p : mParticlesContainer) {

        if (p.life > 0.0) {
            shaderSetVec4(shaderProgram, "inColor", p.color);
            shaderSetVec3(shaderProgram, "inPosition", p.position);
            shaderSetFloat(shaderProgram, "inSize", p.size);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            active_p++;
        }
    }
    // std::cout << active_p << " of " << mNumParticles << " active particles.\n";

    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

}



void FlameEmitter::updateParticles(float dt)
{
    for (Particle& p : mParticlesContainer) {
        p.life -= dt;

        if (p.life > 0.0) {
            float lifeSpan = p.life / mParticleLifetime;    // % of life remaining, [1, 0]
            p.position += p.velocity * float(dt);
            p.velocity.y += 0.5f * dt;
            p.color.a = std::min(p.color.a, lifeSpan);
            p.size = std::min(p.size, (lifeSpan * 2) * mSize);
        }
        else {
            resetParticle(p);
        }
    }

    // Spawn new particles:
    for (int i = 0; i < int(mNumNewParticles * dt); i++) {
        int p_idx = findUnusedParticle();
        Particle& p = mParticlesContainer[p_idx];

        float angle = glm::linearRand(0.0f, 2.0f * glm::pi<float>());
        float distance = glm::linearRand(0.0f, mRadius);

        p.position = {
            cos(angle) * distance,
            0.0f,
            sin(angle) * distance
        };

        p.velocity = {
            glm::linearRand(-0.2f, 0.2f), // x
            glm::linearRand(0.2f, 1.0f),  // y
            glm::linearRand(-0.2f, 0.2f)  // z
        };

        p.color = {
            glm::linearRand(0.7f, 1.0f),
            glm::linearRand(0.1f, 0.7f),
            0.0f,
            0.4f
        };

        p.life = mParticleLifetime;
        p.position += mPosition;
    }
}



void SmokeEmitter::updateParticles(float dt)
{
    for (Particle& p : mParticlesContainer) {
        p.life -= dt;

        if (p.life > 0.0) {
            float lifeSpan = p.life / mParticleLifetime;    // % of life remaining, [1, 0]
            float color = p.color.r;
            color = std::max(color, (1.0f - lifeSpan - 0.1f));

            p.position += p.velocity * float(dt);
            p.velocity.y = std::max(p.velocity.y - 0.12f * dt, 0.01f);

            p.color.r = color;
            p.color.g = color;
            p.color.b = color;
            p.color.a = std::min(p.color.a, lifeSpan);
            p.size = std::max(p.size, (1.0f - lifeSpan) * 4.0f * mSize);
        }
        else {
            resetParticle(p);
        }
    }

    // Spawn new particles:
    for (int i = 0; i < int(mNumNewParticles * dt); i++) {
        int p_idx = findUnusedParticle();
        Particle& p = mParticlesContainer[p_idx];

        float angle = glm::linearRand(0.0f, 2.0f * glm::pi<float>());
        float distance = glm::linearRand(0.0f, mRadius);
        float color = glm::linearRand(0.2f, 0.5f);

        p.position = {
            cos(angle) * distance,
            0.0f,
            sin(angle) * distance
        };

        p.velocity = {
            glm::linearRand(-0.1f, 0.1f), // x
            glm::linearRand(1.0f, 0.8f),  // y
            glm::linearRand(-0.1f, 0.1f)  // z
        };

        p.color = {
            color,
            color,
            color,
            0.4f
        };

        p.life = mParticleLifetime;
        p.position += mPosition;
    }

}



TrailEmitter::TrailEmitter(float timeBetween, float particleLifetime, float particleSize, GLuint texture)
{
    mTimeBetweenParticles = timeBetween;
    mTimeSinceLast = timeBetween;
    mParticleLifetime = particleLifetime;
    mSize = particleSize;
    mTexture = texture;

    mColor = glm::vec4{1.0f, 0.0f, 0.0f, 0.5f };

    initializeParticles();
}

void TrailEmitter::initializeParticles()
{
    mNumParticles = mParticleLifetime / mTimeBetweenParticles * 1.1;
    mParticlesContainer.resize(mNumParticles, Particle());

    fillBuffers();
}

void TrailEmitter::updateParticles(float dt)
{
    for (Particle& p : mParticlesContainer) {
        p.life -= dt;

        if (p.life > 0.0) {
            float lifeSpan = p.life / mParticleLifetime;    // % of life remaining, [1, 0]

            p.color.a = std::min(p.color.a, lifeSpan);
        }
        else {
            resetParticle(p);
        }
    }

    // Spawn new particles:
    if (mTimeSinceLast >= mTimeBetweenParticles) {
        mTimeSinceLast -= mTimeBetweenParticles;

        int p_idx = findUnusedParticle();
        Particle& p = mParticlesContainer[p_idx];

        p.color = {
            mColor.r,
            mColor.g,
            mColor.b,
            mColor.a
        };

        p.life = mParticleLifetime;
        p.position += mPosition;
    }
    mTimeSinceLast += dt;
}
