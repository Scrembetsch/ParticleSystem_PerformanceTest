#pragma once

#include "fs_i_module.h"

class FsModuleEmission : public FsIModule
{
public:
	explicit FsModuleEmission(FsParticleSystem* particleSystem);
	FsModuleEmission(FsParticleSystem* particleSystem, float emitRate);

	float EmitRate;

	void PreRun(float currentTime, float deltaTime) override;
	std::string GetUniforms() const override;
	std::string GetMethodCall() const override;
	std::string GetModuleMethods() const override;
	void ApplyShaderValues(float currentTime, float deltaTime, Shader* shader) override;

private:
	float mCurrentGenerateOffset;
};