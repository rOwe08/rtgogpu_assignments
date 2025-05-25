#include "particle_system.h"
#include "ogl_resource.hpp"
#include "error_handling.hpp"
#include <GLFW/glfw3.h>
#include <random>
#include <algorithm>
#include <ogl_geometry_factory.hpp>
#include "vertex.hpp"
#include <array>
#include <memory>

#include <vector>
#include "mesh_object.hpp"
#include "ogl_geometry_construction.hpp"


ParticleSystem::ParticleSystem(unsigned int amount)
    : mMaxParticles(amount), m_randomGen(std::random_device{}())
{
    mParticles.resize(mMaxParticles);
    for (auto& p : mParticles)
    {
        p.mLife = 0.0f;
    }
    mBuffers = generateParticleBuffers(mParticles);
}

void ParticleSystem::update(float dt, glm::vec3 emitterPos)
{
    int activeParticles = 0;
    
    for (auto& p : mParticles)
    {
        if (p.mLife > 0.0f)
        {
            float lifeNorm = p.mLife / p.mInitialLife;
            // Color smoothly transitions from white -> yellow -> orange -> red
            // We'll use a simple lerp between these colors based on lifeNorm
            glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 yellow = glm::vec3(1.0f, 1.0f, 0.0f);
            glm::vec3 orange = glm::vec3(1.0f, 0.5f, 0.0f);
            glm::vec3 red = glm::vec3(1.0f, 0.1f, 0.0f);
            glm::vec3 color;
            if (lifeNorm > 0.75f) {
                float t = (lifeNorm - 0.75f) / 0.25f;
                color = glm::mix(yellow, white, t); // white to yellow
            } else if (lifeNorm > 0.5f) {
                float t = (lifeNorm - 0.5f) / 0.25f;
                color = glm::mix(orange, yellow, t); // yellow to orange
            } else {
                float t = lifeNorm / 0.5f;
                color = glm::mix(red, orange, t); // orange to red
            }
            p.mColor = glm::vec4(color, lifeNorm); // alpha fades out
            // Apply physics
            p.mPosition += p.mVelocity * dt;
            p.mVelocity += glm::vec3(0.0f, 0.2f, 0.0f) * dt; // Reduced gravity
            p.mLife -= dt * 1.5f;
            p.mColor.a = std::max(0.0f, p.mColor.a - dt * 2.5f); // Prevent negative alpha
            
            activeParticles++;
        }
        else
        {
            // Respawn dead particles
            respawnParticle(p, emitterPos);
        }
    }
    
    // Update GPU buffers
    if (activeParticles > 0) {
        // Regenerate geometry with updated particles
        for (auto& mode : mRenderInfos) {
            mode.second.geometry = std::make_shared<OGLGeometry>(generateParticleBuffers(mParticles));
        }
    }
}

std::shared_ptr<AGeometry> ParticleSystem::getGeometry(GeometryFactory& aGeometryFactory, RenderStyle aRenderStyle)
{
    return std::make_shared<OGLGeometry>(generateParticleBuffers(mParticles));
}

void ParticleSystem::prepareRenderData(MaterialFactory& matFactory, GeometryFactory& geoFactory)
{
    for (auto& mode : mRenderInfos)
    {
        mode.second.shaderProgram = matFactory.getShaderProgram(mode.second.materialParams.mMaterialName);
        getTextures(mode.second.materialParams.mParameterValues, matFactory);
        mode.second.geometry = getGeometry(geoFactory, mode.second.materialParams.mRenderStyle);
    }
}

void ParticleSystem::updateCameraVectors(const glm::mat4& viewMatrix)
{
    // Extract camera right and up vectors from view matrix
    glm::vec3 cameraRight = glm::normalize(glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
    glm::vec3 cameraUp = glm::normalize(glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));

    // Update shader parameters
    for (auto& mode : mRenderInfos)
    {
        if (mode.second.shaderProgram)
        {
            mode.second.materialParams.mParameterValues["u_cameraRight"] = cameraRight;
            mode.second.materialParams.mParameterValues["u_cameraUp"] = cameraUp;
        }
    }
}

unsigned int ParticleSystem::firstUnusedParticle()
{
    for (unsigned int i = mLastUsedParticle; i < mMaxParticles; ++i)
    {
        if (mParticles[i].mLife <= 0.0f)
        {
            mLastUsedParticle = i;
            return i;
        }
    }
    for (unsigned int i = 0; i < mLastUsedParticle; ++i)
    {
        if (mParticles[i].mLife <= 0.0f)
        {
            mLastUsedParticle = i;
            return i;
        }
    }
    mLastUsedParticle = 0;
    return 0;
}

void ParticleSystem::respawnParticle(Particle& p, glm::vec3 emitterPos)
{
    // Generate random angle and radius for circular distribution
    float angle = m_dist(m_randomGen) * 2.0f * 3.14159f;  // random angle from 0 to 2π
    float radius = m_dist(m_randomGen) * 0.5f;  // random radius from 0 to 0.5
    
    // Convert polar coordinates to Cartesian for x and y
    float x = radius * cos(angle);
    float y = radius * sin(angle);
    
    // Generate random height in cylinder
    float z = m_dist(m_randomGen) * 0.5f;  // cylinder height from -0.5 to 0.5

    p.mPosition = emitterPos + glm::vec3(x, y, z);

    // Fire: strong upward, more random horizontal, less gravity
    p.mVelocity = glm::vec3(
        float((rand() % 200) - 100) * 0.001f,  // More random x
        1.8f + m_dist(m_randomGen) * 0.7f,     // Strong upward
        float((rand() % 200) - 100) * 0.002f   // Increased Z movement
    );

    p.mInitialLife = p.mLife = 1.0f + m_dist(m_randomGen) * 0.7f;
    p.mScale = 0.05f + m_dist(m_randomGen) * 0.02f; // Vary particle size for more depth
}

IndexedBuffer ParticleSystem::generateParticleBuffers(const std::vector<Particle>& particles)
{
    IndexedBuffer buffers{ createVertexArray() };
    buffers.vbos.reserve(3);

    // Create quad vertices for particles
    float particleSize = 0.035f;
    std::vector<VertexNormTex> vertices = {
        VertexNormTex(glm::vec3( particleSize,  particleSize, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        VertexNormTex(glm::vec3(-particleSize,  particleSize, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
        VertexNormTex(glm::vec3( particleSize, -particleSize, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        VertexNormTex(glm::vec3(-particleSize, -particleSize, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f))
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    GL_CHECK(glBindVertexArray(buffers.vao.get()));

    // Vertex buffer
    buffers.vbos.push_back(createBuffer());
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers.vbos[0].get()));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexNormTex), vertices.data(), GL_STATIC_DRAW));

    // Element buffer
    buffers.vbos.push_back(createBuffer());
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.vbos[1].get()));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW));

    // Position attribute (location 0)
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), (void*)0));

    // Normal attribute (location 1)
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), (void*)(sizeof(glm::vec3))));

    // Texture coordinate attribute (location 2)
    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), (void*)(2*sizeof(glm::vec3))));

    // Instance buffer
    buffers.vbos.push_back(createBuffer());
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers.vbos[2].get()));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW));

    // Position instance attribute (location 3)
    GL_CHECK(glEnableVertexAttribArray(3));
    GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, mPosition)));
    GL_CHECK(glVertexAttribDivisor(3, 1));

    // Color instance attribute (location 4)
    GL_CHECK(glEnableVertexAttribArray(4));
    GL_CHECK(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, mColor)));
    GL_CHECK(glVertexAttribDivisor(4, 1));

    buffers.mode = GL_TRIANGLE_STRIP;
    buffers.indexCount = static_cast<unsigned>(indices.size());
    buffers.instanceCount = static_cast<unsigned>(particles.size());
    
    GL_CHECK(glBindVertexArray(0));
    return buffers;
}