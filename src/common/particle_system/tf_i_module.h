#pragma once

#include "tf_particle.h"
#include "../gl/shader.h"

class TfParticleSystem;

class TfIModule
{
public:
	TfIModule(TfParticleSystem* particleSystem)
		: ParticleSystem(particleSystem)
	{
	}

	virtual std::string GetUniforms() = 0;
	virtual std::string GetMethodCall(Shader::ShaderType shaderType) = 0;
	virtual std::string GetModuleMethods(Shader::ShaderType shaderType) = 0;
	virtual void ApplyUniforms(float deltaTime, Shader* shader) = 0;

	TfParticleSystem* ParticleSystem;
};