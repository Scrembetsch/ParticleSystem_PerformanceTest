#pragma once

#include "cpu_i_module.h"

class CpuModuleEmission : public CpuIModule
{
public:
	CpuModuleEmission(CpuParticleSystem* particleSystem);
	CpuModuleEmission(CpuParticleSystem* particleSystem, float emitRate);

	float EmitRate;

	void PreRun(float deltaTime) override;
	void UpdateParticle(float deltaTime, Particle& particle) override;

private:
	float mCurrentGenerateOffset;
};