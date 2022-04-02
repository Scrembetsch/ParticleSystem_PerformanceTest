#pragma once

#include "cpu_i_module.h"

class CpuModuleVelOverLife : public CpuIModule
{
public:
	CpuModuleVelOverLife(CpuParticleSystem* particleSystem);
	CpuModuleVelOverLife(CpuParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd);

	void PreRun(float deltaTime) override;
	void UpdateParticle(float deltaTime, Particle& particle) override;

	glm::vec3 VelocityBegin;
	glm::vec3 VelocityEnd;
private:
};