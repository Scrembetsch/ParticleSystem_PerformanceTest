#include "cpu_module_velocity_over_lifetime.h"

#include "cpu_particle_system.h"

CpuModuleVelOverLife::CpuModuleVelOverLife(CpuParticleSystem* particleSystem)
	: CpuModuleVelOverLife(particleSystem, glm::vec3(0.0f), glm::vec3(0.0f))
{
}

CpuModuleVelOverLife::CpuModuleVelOverLife(CpuParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd)
	: CpuIModule(particleSystem)
	, VelocityBegin(velBegin)
	, VelocityEnd(velEnd)
{
	SortId = 1;
}

void CpuModuleVelOverLife::PreRun(float deltaTime)
{
}

void CpuModuleVelOverLife::UpdateParticle(float deltaTime, Particle& particle)
{
	if (!Active | !particle.Active)
		return;

	float t = 1 - (particle.Lifetime / particle.BeginLifetime);
	glm::vec3 vel = VelocityBegin + t * (VelocityEnd - VelocityBegin);

	particle.Velocity += vel * deltaTime;
	particle.Position += particle.Velocity * deltaTime;
}