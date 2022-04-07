#pragma once

#include "cpu_i_particle_system.h"

#include <thread>

class CpuParallelParticleSystem : public CpuIParticleSystem
{
private:
    class Worker
    {
    public:
        void Init(std::vector<CpuIModule*>* modules, std::vector<Particle>* particles, uint32_t threadIndex);
        void StartUpdateParticles(size_t startIndex, size_t endIndex, const glm::vec3& cameraPos, float deltaTime);

        void Join();
        uint32_t GetRemovedParticles() const;

    private:
        void UpdateParticles();

        std::thread mWorkerThread;
        std::vector<CpuIModule*>* mModules;
        std::vector<Particle>* mParticles;

        uint32_t mRemovedParticles;
        uint32_t mThreadId;

        size_t mStartIndex;
        size_t mEndIndex;
        glm::vec3 mCameraPos;
        float mDeltaTime;
    };

public:
    CpuParallelParticleSystem(uint32_t maxParticles, uint32_t threads);
    ~CpuParallelParticleSystem();

    bool Init() override;

    void UpdateParticles(float deltaTime, const glm::vec3& cameraPos) override;
    void RenderParticles() override;

protected:
    void BuildParticleVertexData();

    uint32_t mVao;
    uint32_t mVbo;

    std::vector<Worker> mWorkers;
};