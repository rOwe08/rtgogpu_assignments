#pragma once

#include "mesh_object.hpp"
#include "ogl_geometry_construction.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <random>
#include "vertex.hpp"

class ParticleSystem : public MeshObject
{
public:
    struct Particle
    {
        glm::vec3 mPosition;
        glm::vec3 mVelocity;
        glm::vec4 mColor;
        float mLife;
        float mScale;
        float mInitialLife;
    };

    explicit ParticleSystem(unsigned int amount = 1000);

    void update(float dt, glm::vec3 emitterPos = glm::vec3(0.0f));
    void updateCameraVectors(const glm::mat4& viewMatrix);
    std::shared_ptr<AGeometry> getGeometry(GeometryFactory& factory, RenderStyle style) override;
    void prepareRenderData(MaterialFactory& matFactory, GeometryFactory& geoFactory) override;

    IndexedBuffer mBuffers;
    GLuint m_instanceVBO;
    std::mt19937 m_randomGen;
    std::uniform_real_distribution<float> m_dist{ -0.5f, 0.5f };

private:
    void init();

    static IndexedBuffer generateParticleBuffers(const std::vector<Particle>& particles);

    std::vector<Particle> mParticles;
    unsigned int mMaxParticles;
    unsigned int mLastUsedParticle = 0;

    unsigned int firstUnusedParticle();
    void respawnParticle(Particle& particle, glm::vec3 emitterPos);
};