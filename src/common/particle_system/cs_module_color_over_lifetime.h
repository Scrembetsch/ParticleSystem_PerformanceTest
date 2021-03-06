#pragma once

#include "cs_i_module.h"

class CsModuleColorOverLife : public CsIModule
{
public:
	CsModuleColorOverLife(CsParticleSystem* particleSystem);
	CsModuleColorOverLife(CsParticleSystem* particleSystem, const glm::vec4& ColorBegin, const glm::vec4& ColorEnd);

	const std::vector<std::string>& GetAtomicCounterNames() const override;
	uint32_t GetRequiredAtomics() const override;
	std::string GetUniforms() const override;
	std::string GetMethodCall() const override;
	std::string GetModuleMethods() const override;
	void ReadbackAtomicData(uint32_t* mappedAtomics) override;
	void ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics) override;

	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;
private:
};