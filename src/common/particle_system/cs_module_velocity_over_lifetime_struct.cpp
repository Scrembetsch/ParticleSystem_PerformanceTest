#include "cs_module_velocity_over_lifetime_struct.h"

#include "cs_particle_system_struct.h"

static const std::string sMethodCall = {
"	VelOverlifetimeModule();\n"
};

static const std::string sMethod = {
"void VelOverlifetimeModule()\n"
"{\n"
"  if(lParticle.Lifetime.x >= 0.0)\n"
"  {\n"
"    float t = 1.0 - (lParticle.Lifetime.x / lParticle.Lifetime.y);\n"
"    vec3 vel = uVelocityBegin + t * (uVelocityEnd - uVelocityBegin);\n"
"    lParticle.Velocity.xyz += vel * uDeltaTime;\n"
"  }\n"
"}\n"
};

static const std::vector<std::string> sAtomics;
static const char sUniforms[] =
"uniform vec3 uVelocityBegin;\n"
"uniform vec3 uVelocityEnd;\n";

CsModuleVelOverLifeStruct::CsModuleVelOverLifeStruct(CsParticleSystemStruct* particleSystem)
	: CsModuleVelOverLifeStruct(particleSystem, glm::vec3(0.0f), glm::vec3(0.0f))
{
}

CsModuleVelOverLifeStruct::CsModuleVelOverLifeStruct(CsParticleSystemStruct* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd)
	: CsIModuleStruct(particleSystem)
	, VelocityBegin(velBegin)
	, VelocityEnd(velEnd)
{
}

const std::vector<std::string>& CsModuleVelOverLifeStruct::GetAtomicCounterNames() const
{
	return sAtomics;
}

uint32_t CsModuleVelOverLifeStruct::GetRequiredAtomics() const
{
	return static_cast<uint32_t>(sAtomics.size());
}

std::string CsModuleVelOverLifeStruct::GetMethodCall() const
{
	return sMethodCall;
}

std::string CsModuleVelOverLifeStruct::GetModuleMethods() const
{
	return sMethod;
}

std::string CsModuleVelOverLifeStruct::GetUniforms() const
{
	return sUniforms;
}

void CsModuleVelOverLifeStruct::ReadbackAtomicData(uint32_t* mappedAtomics)
{
}

void CsModuleVelOverLifeStruct::ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics)
{
	shader->SetVec3("uVelocityBegin", VelocityBegin);
	shader->SetVec3("uVelocityEnd", VelocityEnd);
}
