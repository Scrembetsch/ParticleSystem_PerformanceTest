#pragma once

#include "tf_i_module.h"

class TfModuleEmission : public TfIModule
{
public:
	TfModuleEmission(TfParticleSystem* particleSystem);
	TfModuleEmission(TfParticleSystem* particleSystem, float emitRate);

	std::string GetUniforms() override;
	std::string GetMethodCall(Shader::ShaderType shaderType) override;
	std::string GetModuleMethods(Shader::ShaderType shaderType) override;
	void ApplyUniforms(float deltaTime, Shader* shader) override;

	float EmitRate;
private:
	float mCurrentGenerateOffset;
};