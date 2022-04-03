#pragma once

#include "cpu_particle.h"

#include <vector>

class CpuIParticleSystem;

class CpuIModule
{
public:
	CpuIModule(CpuIParticleSystem* particleSystem)
		: ParticleSystem(particleSystem)
	{
	}

	virtual void PreRun(float deltaTime) = 0;
	virtual void UpdateParticle(float deltaTime, Particle& particle) = 0;

	bool Active = true;
	CpuIParticleSystem* ParticleSystem;
	int32_t SortId;
};