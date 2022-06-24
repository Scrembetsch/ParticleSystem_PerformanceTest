#pragma once

#include "cpu_i_module.h"

class CpuModuleEmission : public CpuIModule
{
public:
	explicit CpuModuleEmission(CpuIParticleSystem* particleSystem);
	CpuModuleEmission(CpuIParticleSystem* particleSystem, float emitRate);

	float EmitRate;

	void PreRun(float deltaTime) override;
	void UpdateParticle(float deltaTime, Particle& particle, uint32_t index) override;

private:
	float mCurrentGenerateOffset;
};