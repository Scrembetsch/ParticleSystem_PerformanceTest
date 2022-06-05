#include "fs_module_velocity_over_lifetime.h"
#include "fs_particle_system.h"

static const char sMethodCall[] = "VelocityOverLifeModule();";
static const char sMethod[] = "void VelocityOverLifeModule()\n"
"{\n"
"  if(lParticle.Alive)\n"
"  {\n"
"    vec3 vel = uVelocityBegin + (1.0 - lParticle.TimeT) * (uVelocityEnd - uVelocityBegin);\n"
"    lParticle.Velocity += vel * uDeltaTime;\n"
"  }\n"
"}\n";

static const char sUniforms[] =
"uniform vec3 uVelocityBegin;\n"
"uniform vec3 uVelocityEnd;\n";

FsModuleVelocityOverLifetime::FsModuleVelocityOverLifetime(FsParticleSystem* particleSystem)
	: FsModuleVelocityOverLifetime(particleSystem, glm::vec3(1.0f), glm::vec3(1.0f))
{
}

FsModuleVelocityOverLifetime::FsModuleVelocityOverLifetime(FsParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd)
	: FsIModule(particleSystem)
	, VelocityBegin(velBegin)
	, VelocityEnd(velEnd)

{
}

void FsModuleVelocityOverLifetime::PreRun(float currentTime, float deltaTime)
{
}

std::string FsModuleVelocityOverLifetime::GetUniforms() const
{
	return sUniforms;
}

std::string FsModuleVelocityOverLifetime::GetMethodCall() const
{
	return sMethodCall;
}

std::string FsModuleVelocityOverLifetime::GetModuleMethods() const
{
	return sMethod;
}

void FsModuleVelocityOverLifetime::ApplyShaderValues(float currentTime, float deltaTime, Shader* shader)
{
	shader->SetVec3("uVelocityBegin", VelocityBegin);
	shader->SetVec3("uVelocityEnd", VelocityEnd);
}