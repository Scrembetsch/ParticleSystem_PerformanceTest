#pragma once

#include "fs_i_module.h"

class FsModuleVelocityOverLifetime : public FsIModule
{
public:
	explicit FsModuleVelocityOverLifetime(FsParticleSystem* particleSystem);
	FsModuleVelocityOverLifetime(FsParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd);

	glm::vec3 VelocityBegin;
	glm::vec3 VelocityEnd;

	void PreRun(float currentTime, float deltaTime) override;
	std::string GetUniforms() const override;
	std::string GetMethodCall() const override;
	std::string GetModuleMethods() const override;
	void ApplyShaderValues(float currentTime, float deltaTime, Shader* shader) override;

private:
};