#pragma once

#include "cs_particle.h"
#include "../gl/shader.h"
#include "../gl/gl_util.h"

class CsParticleSystemStruct;

class CsIModuleStruct
{
public:
	CsIModuleStruct(CsParticleSystemStruct* particleSystem)
		: ParticleSystem(particleSystem)
	{
	}

	virtual const std::vector<std::string>& GetAtomicCounterNames() const = 0;
	virtual uint32_t GetRequiredAtomics() const = 0;
	virtual std::string GetUniforms() const = 0;
	virtual std::string GetMethodCall() const = 0;
	virtual std::string GetModuleMethods() const = 0;
	virtual void ReadbackAtomicData(uint32_t* mappedAtomics) = 0;
	virtual void ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics) = 0;

	CsParticleSystemStruct* ParticleSystem;
};