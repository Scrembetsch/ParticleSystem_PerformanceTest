#pragma once

#include "../glm/glm.hpp"
#include "../util/random.h"

#include <thread>
#include <vector>

struct Particle
{
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;

    float Lifetime = 0.0f;
    float Active = false;
    uint32_t Seed = 0;
};

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

    bool Init() ;

    void UpdateParticles(float deltaTime);
    void RenderParticles();

protected:
    void InitParticles();
    void BuildParticleVertexData();
    static const uint32_t sBufferSize = 1;
    uint32_t mVao[sBufferSize];
    uint32_t mVbo[sBufferSize];

    uint32_t mCurrentBuffer;

    uint32_t mNumMaxParticles;
    uint32_t mNumParticles;

    Random mRandom;

    std::vector<float> mParticleRenderData;
    std::vector<Particle> mParticles;
    std::vector<ParticleThread> mParticleThreads;
};