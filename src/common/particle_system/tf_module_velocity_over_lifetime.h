#pragma once

#include "tf_i_module.h"

class TfModuleVelOverLife : public TfIModule
{
public:
	TfModuleVelOverLife(TfParticleSystem* particleSystem);
	TfModuleVelOverLife(TfParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd);

	std::string GetUniforms() override;
	std::string GetMethodCall(Shader::ShaderType shaderType) override;
	std::string GetModuleMethods(Shader::ShaderType shaderType) override;
	void ApplyUniforms(float deltaTime, Shader* shader) override;

	glm::vec3 VelocityBegin;
	glm::vec3 VelocityEnd;
private:
};