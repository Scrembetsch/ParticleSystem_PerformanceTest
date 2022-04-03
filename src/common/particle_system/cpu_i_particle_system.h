#pragma once

#include "cpu_i_module.h"

#include "../util/random.h"

#include <thread>
#include <vector>
#include "../gl/camera.h"

class CpuModuleEmission;

class CpuIParticleSystem
{
public:
    CpuIParticleSystem() = default;
    virtual ~CpuIParticleSystem() = default;

    virtual bool Init() = 0;

    virtual void UpdateParticles(float deltaTime, const glm::vec3& cameraPos) = 0;
    virtual void RenderParticles() = 0;

    virtual void Emit(uint32_t numToGenerate) = 0;

    virtual bool AddModule(CpuIModule* cpuModule) = 0;

    virtual uint32_t GetCurrentParticles() const = 0;

    virtual void SetMinLifetime(float minLifetime) = 0;
    virtual void SetMaxLifetime(float maxLifetime) = 0;

    virtual void SetMinStartVelocity(const glm::vec3& minVelocity) = 0;
    virtual void SetMaxStartVelocity(const glm::vec3& maxVelocity) = 0;

protected:
    static const uint32_t mNumVertices = 6;
};