#pragma once
#include "tf_particle_system.h"

class TfFullEmitParticleSystem : public TfParticleSystem
{
public:
    explicit TfFullEmitParticleSystem(uint32_t maxParticles, float emitRate);
    ~TfFullEmitParticleSystem() = default;

    virtual bool Init();
    virtual void UpdateParticles(float timeStep, const glm::vec3& cameraPos);

    float EmitRate;
protected:
    Shader mEmitShader;
    int32_t mMaxComponents;
    int32_t mComponentSize;
    uint32_t mMaxVerticesPerPass;

    float mCurrentGenerateOffset;
};