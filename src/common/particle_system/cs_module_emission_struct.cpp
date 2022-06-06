#include "cs_module_emission_struct.h"

#include "cs_particle_system_struct.h"

static const std::string sMethodCall = {
"	if(lParticle.Lifetime.x <= 0.0)\n"
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

CsModuleEmissionStruct::CsModuleEmissionStruct(CsParticleSystemStruct* particleSystem)
	: CsModuleEmissionStruct(particleSystem, 0.0f)
{
}

CsModuleEmissionStruct::CsModuleEmissionStruct(CsParticleSystemStruct* particleSystem, float emitRate)
	: CsIModuleStruct(particleSystem)
	, EmitRate(emitRate)
	, mCurrentGenerateOffset(0.0f)
{
}

const std::vector<std::string>& CsModuleEmissionStruct::GetAtomicCounterNames() const
{
	return sAtomics;
}

uint32_t CsModuleEmissionStruct::GetRequiredAtomics() const
{
	return static_cast<uint32_t>(sAtomics.size());
}

std::string CsModuleEmissionStruct::GetMethodCall() const
{
	return sMethodCall;
}

std::string CsModuleEmissionStruct::GetModuleMethods() const
{
	return sMethod;
}

std::string CsModuleEmissionStruct::GetUniforms() const
{
	return sUniforms;
}

void CsModuleEmissionStruct::ReadbackAtomicData(uint32_t* mappedAtomics)
{
}

void CsModuleEmissionStruct::ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics)
{
	float timeForOneCpuParticle = 1.0f / EmitRate;

	mCurrentGenerateOffset += deltaTime;
	uint32_t numToGenerate = 0;

	numToGenerate = static_cast<uint32_t>(mCurrentGenerateOffset / timeForOneCpuParticle);
	mCurrentGenerateOffset -= static_cast<float>(numToGenerate) * timeForOneCpuParticle;

	mappedAtomics[ParticleSystem->GetAtomicLocation(sAtomics[0])] = numToGenerate;
}
