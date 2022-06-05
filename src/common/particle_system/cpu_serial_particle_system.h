#pragma once

#include "cpu_i_particle_system.h"

class CpuSerialParticleSystem : public CpuIParticleSystem
{
public:
    CpuSerialParticleSystem(uint32_t maxParticles);
    ~CpuSerialParticleSystem();

    bool Init() override;

    void UpdateParticles(float deltaTime, const glm::vec3& cameraPos) override;
    void RenderParticles() override;

protected:
    void BuildParticleVertexData();

    uint32_t mVao;
    uint32_t mVeo;
    uint32_t mVbo;

#if INDEXED
    std::vector<uint32_t> mIndices;
#endif

    std::vector<CpuRenderParticle> mParticleRenderData;
};