#include "cs_module_emission.h"

#include "cs_particle_system.h"

static const std::string sMethodCall = {
"	if(!lParticle.Alive)\n"
"	{\n"
"		EmissionModule();\n"
"   }\n"
};

static const std::string sMethod = {
"void EmissionModule()\n"
"{\n"
"  if(atomicCounter(EmitNumToGenerate) < (-1U / 2U))"
"  {"
"    if(atomicCounterDecrement(EmitNumToGenerate) < (-1U / 2U))\n"
"    {\n"
"        InitParticle();\n"
"     }\n"
"  }\n"
"}\n"
};

static const std::vector<std::string> sAtomics = { "EmitNumToGenerate" };
static const char sUniforms[] = " ";

CsModuleEmission::CsModuleEmission(CsParticleSystem* particleSystem)
	: CsModuleEmission(particleSystem, 0.0f)
{
}

CsModuleEmission::CsModuleEmission(CsParticleSystem* particleSystem, float emitRate)
	: CsIModule(particleSystem)
	, EmitRate(emitRate)
	, mCurrentGenerateOffset(0.0f)
{
}

const std::vector<std::string>& CsModuleEmission::GetAtomicCounterNames() const
{
	return sAtomics;
}

uint32_t CsModuleEmission::GetRequiredAtomics() const
{
	return static_cast<uint32_t>(sAtomics.size());
}

std::string CsModuleEmission::GetMethodCall() const
{
	return sMethodCall;
}

std::string CsModuleEmission::GetModuleMethods() const
{
	return sMethod;
}

std::string CsModuleEmission::GetUniforms() const
{
	return sUniforms;
}

void CsModuleEmission::ReadbackAtomicData(uint32_t* mappedAtomics)
{
}

void CsModuleEmission::ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics)
{
	float timeForOneCpuParticle = 1.0f / EmitRate;

	mCurrentGenerateOffset += deltaTime;
	uint32_t numToGenerate = 0;

	numToGenerate = static_cast<uint32_t>(mCurrentGenerateOffset / timeForOneCpuParticle);
	mCurrentGenerateOffset -= static_cast<float>(numToGenerate) * timeForOneCpuParticle;

	mappedAtomics[ParticleSystem->GetAtomicLocation(sAtomics[0])] = numToGenerate;
}
