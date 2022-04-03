#pragma once

#include "cpu_i_particle_system.h"

class CpuSerialInstanceParticleSystem : public CpuIParticleSystem
{
public:
    CpuSerialInstanceParticleSystem(uint32_t maxParticles);
    ~CpuSerialInstanceParticleSystem();

    bool Init() override;

    void UpdateParticles(float deltaTime, const glm::vec3& cameraPos) override;
    void RenderParticles() override;

    void Emit(uint32_t numToGenerate) override;

    bool AddModule(CpuIModule* cpuModule) override;

    uint32_t GetCurrentParticles() const override;

    void SetMinLifetime(float minLifetime) override;
    void SetMaxLifetime(float maxLifetime) override;

    void SetMinStartVelocity(const glm::vec3& minVelocity) override;
    void SetMaxStartVelocity(const glm::vec3& maxVelocity) override;

protected:
    void InitParticles(uint32_t initFrom, bool active);
    void BuildParticleVertexData();

    void InitParticle(Particle& particle, bool active);

    void SortParticles();

    uint32_t mVao;
    uint32_t mVboParticlePosition;
    uint32_t mVboParticleData;

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