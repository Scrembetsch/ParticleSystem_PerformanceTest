#pragma once

#include "cpu_particle.h"

#include <vector>

class CpuParticleSystem;

class CpuIModule
{
public:
	CpuIModule(CpuParticleSystem* particleSystem)
		: ParticleSystem(particleSystem)
	{
	}

	virtual void PreRun(float deltaTime) = 0;
	virtual void UpdateParticle(float deltaTime, Particle& particle) = 0;

	bool Active = true;
	CpuParticleSystem* ParticleSystem;
	int32_t SortId;
};