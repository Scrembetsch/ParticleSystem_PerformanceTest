#include "cs_module_color_over_lifetime.h"

#include "cs_particle_system.h"

static const std::string sMethodCall = {
"	ColorOverlifetimeModule(color, lifetimes);\n"
};

static const std::string sMethod = {
"void ColorOverlifetimeModule(inout vec4 color, vec2 lifetimes)\n"
"{\n"
"  if(lifetimes.x >= 0.0)\n"
"  {\n"
"    float t = 1.0 - (lifetimes.x / lifetimes.y);\n"
"    color = uColorBegin + t * (uColorEnd - uColorBegin);\n"
"  }\n"
"}\n"
};

static const std::vector<std::string> sAtomics;
static const char sUniforms[] =
"uniform vec4 uColorBegin;\n"
"uniform vec4 uColorEnd;\n";

CsModuleColorOverLife::CsModuleColorOverLife(CsParticleSystem* particleSystem)
	: CsModuleColorOverLife(particleSystem, glm::vec4(0.0f), glm::vec4(0.0f))
{
}

CsModuleColorOverLife::CsModuleColorOverLife(CsParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd)
	: CsIModule(particleSystem)
	, ColorBegin(colorBegin)
	, ColorEnd(colorEnd)
{
}

const std::vector<std::string>& CsModuleColorOverLife::GetAtomicCounterNames() const
{
	return sAtomics;
}

uint32_t CsModuleColorOverLife::GetRequiredAtomics() const
{
	return static_cast<uint32_t>(sAtomics.size());
}

std::string CsModuleColorOverLife::GetMethodCall() const
{
	return sMethodCall;
}

std::string CsModuleColorOverLife::GetModuleMethods() const
{
	return sMethod;
}

std::string CsModuleColorOverLife::GetUniforms() const
{
	return sUniforms;
}

void CsModuleColorOverLife::ReadbackAtomicData(uint32_t* mappedAtomics)
{
}

void CsModuleColorOverLife::ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics)
{
	shader->SetVec4("uColorBegin", ColorBegin);
	shader->SetVec4("uColorEnd", ColorEnd);
}
