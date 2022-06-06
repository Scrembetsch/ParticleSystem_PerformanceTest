#include "cs_module_velocity_over_lifetime.h"

#include "cs_particle_system.h"

static const std::string sMethodCall = {
"	VelOverlifetimeModule();\n"
};

static const std::string sMethod = {
"void VelOverlifetimeModule()\n"
"{\n"
"  vec3 vel = uVelocityBegin + (1.0 - lParticle.LifetimeT) * (uVelocityEnd - uVelocityBegin);\n"
"  lParticle.Velocity += vel * uDeltaTime * lParticle.AliveF;\n"
"}\n"
};

static const std::vector<std::string> sAtomics;
static const char sUniforms[] =
"uniform vec3 uVelocityBegin;\n"
"uniform vec3 uVelocityEnd;\n";

CsModuleVelOverLife::CsModuleVelOverLife(CsParticleSystem* particleSystem)
	: CsModuleVelOverLife(particleSystem, glm::vec3(0.0f), glm::vec3(0.0f))
{
}

CsModuleVelOverLife::CsModuleVelOverLife(CsParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd)
	: CsIModule(particleSystem)
	, VelocityBegin(velBegin)
	, VelocityEnd(velEnd)
{
}

const std::vector<std::string>& CsModuleVelOverLife::GetAtomicCounterNames() const
{
	return sAtomics;
}

uint32_t CsModuleVelOverLife::GetRequiredAtomics() const
{
	return static_cast<uint32_t>(sAtomics.size());
}

std::string CsModuleVelOverLife::GetMethodCall() const
{
	return sMethodCall;
}

std::string CsModuleVelOverLife::GetModuleMethods() const
{
	return sMethod;
}

std::string CsModuleVelOverLife::GetUniforms() const
{
	return sUniforms;
}

void CsModuleVelOverLife::ReadbackAtomicData(uint32_t* mappedAtomics)
{
}

void CsModuleVelOverLife::ApplyShaderValues(float deltaTime, Shader* shader, uint32_t* mappedAtomics)
{
	shader->SetVec3("uVelocityBegin", VelocityBegin);
	shader->SetVec3("uVelocityEnd", VelocityEnd);
}
