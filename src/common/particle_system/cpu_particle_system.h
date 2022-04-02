#pragma once

#include "cpu_i_module.h"

#include "../util/random.h"

#include <thread>
#include <vector>
#include "../gl/camera.h"

class CpuModuleEmission;

class CpuParticleSystem
{
public:
    class ParticleThread
    {
    public:
        ParticleThread();
        ~ParticleThread();

        bool Run();

        void SetData(Particle* start, uint32_t numParticles, float deltaTime);
        uint32_t GetRemovedParticleCount() const;
    protected:
        std::thread mThread;

        Particle* mStartParticle;
        uint32_t mNumModules;
        uint32_t mNumParticles;
        float mDeltaTime;

        uint32_t mRemovedParticles;
    };

    CpuParticleSystem();
    ~CpuParticleSystem();

    bool Init();

    void UpdateParticles(float deltaTime, const glm::vec3& cameraPos);
    void RenderParticles();

    void Emit(uint32_t numToGenerate);

    bool AddModule(CpuIModule* cpuModule);
    CpuModuleEmission* GetEmissionModule();

    uint32_t GetCurrentParticles() const;

    void SetMinLifetime(float minLifetime);
    void SetMaxLifetime(float maxLifetime);

    void SetMinStartVelocity(const glm::vec3& minVelocity);
    void SetMaxStartVelocity(const glm::vec3& maxVelocity);

protected:
    void InitParticles(uint32_t initFrom, bool active);
    void BuildParticleVertexData();

    void InitParticle(Particle& particle, bool active);

    void SortParticles();

    static const uint32_t sBufferSize = 1;
    uint32_t mVao[sBufferSize];
    uint32_t mVbo[sBufferSize];

    uint32_t mCurrentBuffer;

    uint32_t mNumMaxParticles;
    uint32_t mNumParticles;

    Random mRandom;

    float mMinLifetime;
    float mMaxLifetime;

    glm::vec3 mMinStartVelocity;
    glm::vec3 mMaxStartVelocity;

    std::vector<float> mParticleRenderData;
    std::vector<Particle> mParticles;
    std::vector<ParticleThread> mParticleThreads;
    std::vector<CpuIModule*> mModules;
};