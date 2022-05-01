#pragma once
#include "cpu_i_module.h"

#include "../defines.h"
#include "../util/random.h"

#include <thread>
#include <vector>
#include "../gl/camera.h"

class CpuModuleEmission;

class CpuIParticleSystem
{
protected:
    static const uint32_t mNumVertices = 6;
    static const uint32_t mNumIndexedVertices = 4;

public:
    explicit CpuIParticleSystem(uint32_t maxParticles);
    virtual ~CpuIParticleSystem();

    virtual bool Init() = 0;

    virtual void UpdateParticles(float deltaTime, const glm::vec3& cameraPos) = 0;
    virtual void RenderParticles() = 0;

    virtual void Emit(uint32_t numToGenerate);

    virtual bool AddModule(CpuIModule* psModule);

    virtual uint32_t GetCurrentParticles() const;

    virtual void SetMinLifetime(float minLifetime);
    virtual void SetMaxLifetime(float maxLifetime);

    virtual void SetMinStartVelocity(const glm::vec3& minVelocity);
    virtual void SetMaxStartVelocity(const glm::vec3& maxVelocity);

protected:
    virtual void InitParticles(uint32_t initFrom, bool active);
    virtual void InitParticle(Particle& particle, bool active);

    virtual void SortParticles();

    uint32_t mNumMaxParticles;
    uint32_t mNumParticles;

    Random mRandom;

    float mMinLifetime;
    float mMaxLifetime;

    glm::vec3 mMinStartVelocity;
    glm::vec3 mMaxStartVelocity;

    std::vector<float> mParticleRenderData;
    std::vector<Particle> mParticles;
    std::vector<CpuIModule*> mModules;
};