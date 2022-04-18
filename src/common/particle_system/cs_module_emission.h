#pragma once

#include "cs_i_module.h"

class CsModuleEmission : public CsIModule
{
public:
	CsModuleEmission(CsParticleSystem* particleSystem);
	CsModuleEmission(CsParticleSystem* particleSystem, float emitRate);

	const std::vector<std::string>& GetAtomicCounterNames() const override;
	uint32_t GetRequiredAtomics() const override;
	std::string GetUniforms() const override;
	std::string GetMethodCall() const override;
	std::string GetModuleMethods() const override;
	void ReadbackAtomicData(uint32_t* mappedAtomics) override;
	void ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics) override;

	float EmitRate;
private:
	float mCurrentGenerateOffset;
};