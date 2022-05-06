#pragma once

#include "cs_i_module_struct.h"

class CsModuleEmissionStruct : public CsIModuleStruct
{
public:
	CsModuleEmissionStruct(CsParticleSystemStruct* particleSystem);
	CsModuleEmissionStruct(CsParticleSystemStruct* particleSystem, float emitRate);

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