#include "cpu_module_color_over_lifetime.h"

#include "cpu_particle_system.h"

CpuModuleColorOverLife::CpuModuleColorOverLife(CpuParticleSystem* particleSystem)
	: CpuModuleColorOverLife(particleSystem, glm::vec4(0.0f), glm::vec4(0.0f))
{
}

CpuModuleColorOverLife::CpuModuleColorOverLife(CpuParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd)
	: CpuIModule(particleSystem)
	, ColorBegin(colorBegin)
	, ColorEnd(colorEnd)
{
	SortId = 1;
}

void CpuModuleColorOverLife::PreRun(float deltaTime)
{
}

void CpuModuleColorOverLife::UpdateParticle(float deltaTime, Particle& particle)
{
	if (!Active | !particle.Active)
		return;

	float t = 1 - (particle.Lifetime / particle.BeginLifetime);
	glm::vec4 color = ColorBegin + t * (ColorEnd - ColorBegin);

	particle.Color = color;
}