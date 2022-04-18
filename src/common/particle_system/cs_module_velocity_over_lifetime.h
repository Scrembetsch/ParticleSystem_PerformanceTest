#pragma once

#include "cs_i_module.h"

class CsModuleVelOverLife : public CsIModule
{
public:
	CsModuleVelOverLife(CsParticleSystem* particleSystem);
	CsModuleVelOverLife(CsParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd);

	const std::vector<std::string>& GetAtomicCounterNames() const override;
	uint32_t GetRequiredAtomics() const override;
	std::string GetUniforms() const override;
	std::string GetMethodCall() const override;
	std::string GetModuleMethods() const override;
	void ReadbackAtomicData(uint32_t* mappedAtomics) override;
	void ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics) override;

	glm::vec3 VelocityBegin;
	glm::vec3 VelocityEnd;
private:
};