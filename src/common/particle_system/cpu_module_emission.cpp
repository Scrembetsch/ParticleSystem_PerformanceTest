#include "cpu_module_emission.h"

#include "cpu_particle_system.h"

CpuModuleEmission::CpuModuleEmission(CpuParticleSystem* particleSystem)
	: CpuModuleEmission(particleSystem, 0.0f)
{
}

CpuModuleEmission::CpuModuleEmission(CpuParticleSystem* particleSystem, float emitRate)
	: CpuIModule(particleSystem)
	, EmitRate(emitRate)
	, mCurrentGenerateOffset(0.0f)
{
	SortId = 0;
}

void CpuModuleEmission::PreRun(float deltaTime)
{
	if (!Active)
	{
		return;
	}

	float timeForOneCpuParticle = 1.0f / EmitRate;

	mCurrentGenerateOffset += deltaTime;
	uint32_t numToGenerate = 0;

	// Todo: Could improve
	while (mCurrentGenerateOffset >= timeForOneCpuParticle)
	{
		numToGenerate++;
		mCurrentGenerateOffset -= timeForOneCpuParticle;
	}

	ParticleSystem->Emit(numToGenerate);
}

void CpuModuleEmission::UpdateParticle(float deltaTime, Particle& particle)
{
}