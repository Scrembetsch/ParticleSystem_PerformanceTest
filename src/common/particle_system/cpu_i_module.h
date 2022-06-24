#pragma once

#include "cpu_particle.h"

#include <vector>

class CpuIParticleSystem;

class CpuIModule
{
public:
	explicit CpuIModule(CpuIParticleSystem* particleSystem)
		: ParticleSystem(particleSystem)
	{
	}
	virtual ~CpuIModule() = default;

	virtual void PreRun(float deltaTime) = 0;
	virtual void UpdateParticle(float deltaTime, Particle& particle, uint32_t index) = 0;

	bool Active = true;
	CpuIParticleSystem* ParticleSystem;
};