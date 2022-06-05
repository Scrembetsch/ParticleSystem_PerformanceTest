#pragma once

#include "../gl/shader.h"
#include "../gl/gl_util.h"

class FsParticleSystem;

class FsIModule
{
public:
	FsIModule(FsParticleSystem* particleSystem)
		: ParticleSystem(particleSystem)
	{
	}

	virtual void PreRun(float currentTime, float deltaTime) = 0;
	virtual std::string GetUniforms() const = 0;
	virtual std::string GetMethodCall() const = 0;
	virtual std::string GetModuleMethods() const = 0;
	virtual void ApplyShaderValues(float currentTime, float deltaTime, Shader* shader) = 0;

	FsParticleSystem* ParticleSystem;
};