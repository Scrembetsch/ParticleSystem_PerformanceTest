#pragma once

#include "tf_i_module.h"

class TfModuleColorOverLife : public TfIModule
{
public:
	TfModuleColorOverLife(TfParticleSystem* particleSystem);
	TfModuleColorOverLife(TfParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd);

	std::string GetUniforms() override;
	std::string GetMethodCall(Shader::ShaderType shaderType) override;
	std::string GetModuleMethods(Shader::ShaderType shaderType) override;
	void ApplyUniforms(float deltaTime, Shader* shader) override;

	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;
private:
};