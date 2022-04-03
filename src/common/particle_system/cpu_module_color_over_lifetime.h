#pragma once

#include "cpu_i_module.h"

class CpuModuleColorOverLife : public CpuIModule
{
public:
	CpuModuleColorOverLife(CpuIParticleSystem* particleSystem);
	CpuModuleColorOverLife(CpuIParticleSystem* particleSystem, const glm::vec4& colBegin, const glm::vec4& colEnd);

	void PreRun(float deltaTime) override;
	void UpdateParticle(float deltaTime, Particle& particle) override;

	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;
private:
};