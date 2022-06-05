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

protected:
    void BuildParticleVertexData();

    uint32_t mVao;
    uint32_t mVboParticlePosition;
    uint32_t mVboParticleData;

    std::vector<float> mParticleRenderData;

};