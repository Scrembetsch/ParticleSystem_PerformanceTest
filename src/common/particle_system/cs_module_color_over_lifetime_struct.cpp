#include "cs_module_color_over_lifetime_struct.h"

#include "cs_particle_system_struct.h"

static const std::string sMethodCall = {
"	ColorOverlifetimeModule();\n"
};

static const std::string sMethod = {
"void ColorOverlifetimeModule()\n"
"{\n"
"  float t = (1.0 - lParticle.LifetimeT);\n"
"  lParticle.Color = uColorBegin + t * (uColorEnd - uColorBegin);\n"
"}\n"
};

static const std::vector<std::string> sAtomics;
static const char sUniforms[] =
"uniform vec4 uColorBegin;\n"
"uniform vec4 uColorEnd;\n";

CsModuleColorOverLifeStruct::CsModuleColorOverLifeStruct(CsParticleSystemStruct* particleSystem)
	: CsModuleColorOverLifeStruct(particleSystem, glm::vec4(0.0f), glm::vec4(0.0f))
{
}

CsModuleColorOverLifeStruct::CsModuleColorOverLifeStruct(CsParticleSystemStruct* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd)
	: CsIModuleStruct(particleSystem)
	, ColorBegin(colorBegin)
	, ColorEnd(colorEnd)
{
}

const std::vector<std::string>& CsModuleColorOverLifeStruct::GetAtomicCounterNames() const
{
	return sAtomics;
}

uint32_t CsModuleColorOverLifeStruct::GetRequiredAtomics() const
{
	return static_cast<uint32_t>(sAtomics.size());
}

std::string CsModuleColorOverLifeStruct::GetMethodCall() const
{
	return sMethodCall;
}

std::string CsModuleColorOverLifeStruct::GetModuleMethods() const
{
	return sMethod;
}

std::string CsModuleColorOverLifeStruct::GetUniforms() const
{
	return sUniforms;
}

void CsModuleColorOverLifeStruct::ReadbackAtomicData(uint32_t* mappedAtomics)
{
}

void CsModuleColorOverLifeStruct::ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics)
{
	shader->SetVec4("uColorBegin", ColorBegin);
	shader->SetVec4("uColorEnd", ColorEnd);
}
