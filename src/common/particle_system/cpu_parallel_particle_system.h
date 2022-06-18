#pragma once

#include "cpu_i_particle_system.h"

#include <thread>
#include <mutex>

#if PARALLEL
#define USE_WORKERS 0
#define PARALLEL_UPDATE 0
#define PARALLEL_SORT 0
#define PARALLEL_BUILD 0
#endif

// 128 * 128
// SER                      193.201
// PAR (THREAD) (UP)        189.545
// PAR (THREAD) (SORT)      174.664
// PAR (THREAD) (BUILD)     192.737
// PAR (THREAD)             115.653
// PAR (WORKER) (UP)        166.223
// PAR (WORKER) (SORT)      156.375
// PAR (WORKER) (BUILD)     163.764
// PAR (WORKER)             160.692

// 256 * 256
// SER                      59.7502
// PAR (THREAD)             55.9424
// PAR (WORKER)             61.646

class CpuParallelParticleSystem : public CpuIParticleSystem
{
private:
    class Worker
    {
    public:
        Worker(CpuParallelParticleSystem* ps)
            : mParticleSystem(ps)
        {
        }

        enum Job
        {
            NONE,
            JOB_UPDATE,
            JOB_SORT,
            JOB_BUILD
        };

        void Run(uint32_t threadId, const glm::vec3& cameraPos, float deltaTime);
        void WaitForJob();

        void StartJob(Job job, uint32_t startIndex, uint32_t endIndex);

        void Join();
        uint32_t GetRemovedParticles() const;

        static uint32_t Update(std::vector<Particle>& particles, const std::vector<CpuIModule*>& modules, uint32_t startIndex, uint32_t endIndex, const glm::vec3& cameraPos, float deltaTime);
        static void QuickSort(CpuParallelParticleSystem* ps, int32_t begin, int32_t end);
        static void BuildVertices(CpuParallelParticleSystem* ps, int32_t begin, int32_t end);

    private:
        void Run();

        void UpdateParticles();
        void Sort();
        void BuildVertices();
        static int32_t Partition(std::vector<Particle>& particles, int32_t begin, int32_t end);

        CpuParallelParticleSystem* mParticleSystem;

        std::thread mWorkerThread;

        uint32_t mRemovedParticles;
        uint32_t mThreadId;

        size_t mStartIndex;
        size_t mEndIndex;
        glm::vec3 mCameraPos;
        float mDeltaTime;

        Job mCurrentJob;
        std::mutex mJobMutex;
        std::atomic_bool mRunning;
        std::atomic_bool mJobFinished;
    };

public:
    friend Worker;

    CpuParallelParticleSystem(uint32_t maxParticles, uint32_t threads);
    ~CpuParallelParticleSystem();

    bool Init() override;

    void UpdateParticles(float deltaTime, const glm::vec3& cameraPos) override;
    void RenderParticles() override;

protected:
    void BuildParticleVertexData();

    uint32_t mVeo;
    uint32_t mVao;
    uint32_t mVbo;

#if USE_WORKERS
    std::vector<Worker*> mWorkers;
#else
    std::vector<std::thread> mWorkers;
#endif
    std::atomic_uint32_t mParticlesToDraw;

    std::atomic_uint32_t mThreadCounter;
    std::atomic_uint32_t mActiveThreads;

    std::vector<CpuRenderParticle> mParticleRenderData;
#if INDEXED
    std::vector<uint32_t> mIndices;
#endif
};