#pragma once

#include "fs_i_module.h"

class FsModuleColorOverLifetime : public FsIModule
{
public:
	explicit FsModuleColorOverLifetime(FsParticleSystem* particleSystem);
	FsModuleColorOverLifetime(FsParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd);

	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;

	void PreRun(float currentTime, float deltaTime) override;
	std::string GetUniforms() const override;
	std::string GetMethodCall() const override;
	std::string GetModuleMethods() const override;
	void ApplyShaderValues(float currentTime, float deltaTime, Shader* shader) override;

private:
};