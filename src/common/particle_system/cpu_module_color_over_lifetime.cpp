#include "cpu_module_color_over_lifetime.h"

#include "cpu_i_particle_system.h"

CpuModuleColorOverLife::CpuModuleColorOverLife(CpuIParticleSystem* particleSystem)
	: CpuModuleColorOverLife(particleSystem, glm::vec4(0.0f), glm::vec4(0.0f))
{
}

CpuModuleColorOverLife::CpuModuleColorOverLife(CpuIParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd)
	: CpuIModule(particleSystem)
	, ColorBegin(colorBegin)
	, ColorEnd(colorEnd)
{
}

void CpuModuleColorOverLife::PreRun(float deltaTime)
{
}

void CpuModuleColorOverLife::UpdateParticle(float deltaTime, Particle& particle, uint32_t index)
{
	if (!Active | !particle.Active)
		return;

	float t = 1 - (particle.Lifetime / particle.BeginLifetime);
	glm::vec4 color = ColorBegin + t * (ColorEnd - ColorBegin);

	particle.Color = color;
}