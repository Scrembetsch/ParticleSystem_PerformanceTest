#include "tf_module_velocity_over_lifetime.h"

#include "tf_particle_system.h"

static const char sMethodCall[] = "VelOverLifeModule();";
static const char sMethod[] = "void VelOverLifeModule()\n"
"{\n"
"  if(vTypeOut != 0.0)\n"
"  {\n"
"    float t = 1.0 - (vLifeTimeOut / vLifeTimeBeginOut);\n"
"    vec3 vel = uVelocityBegin + t * (uVelocityEnd - uVelocityBegin);\n"
"    vVelocityOut += vel * uTimeStep;\n"
"  }\n"
"}\n";

static const char sUniforms[] =
"uniform vec3 uVelocityBegin;\n"
"uniform vec3 uVelocityEnd;\n";

TfModuleVelOverLife::TfModuleVelOverLife(TfParticleSystem* particleSystem)
	: TfModuleVelOverLife(particleSystem, glm::vec3(0.0f), glm::vec3(0.0f))
{
}

TfModuleVelOverLife::TfModuleVelOverLife(TfParticleSystem* particleSystem, const glm::vec3& velBegin, const glm::vec3& velEnd)
	: TfIModule(particleSystem)
	, VelocityBegin(velBegin)
	, VelocityEnd(velEnd)
{
}

std::string TfModuleVelOverLife::GetMethodCall(Shader::ShaderType shaderType)
{
	switch (shaderType)
	{
	case Shader::ShaderType::SHADER_TYPE_GEOMETRY:
		return sMethodCall;

	default:
		return "";
	}
}

std::string TfModuleVelOverLife::GetModuleMethods(Shader::ShaderType shaderType)
{
	switch (shaderType)
	{
	case Shader::ShaderType::SHADER_TYPE_GEOMETRY:
		return sMethod;

	default:
		return "";
	}
}

std::string TfModuleVelOverLife::GetUniforms()
{
	return sUniforms;
}

void TfModuleVelOverLife::ApplyUniforms(float deltaTime, Shader* shader)
{
	shader->SetVec3("uVelocityBegin", VelocityBegin);
	shader->SetVec3("uVelocityEnd", VelocityEnd);
}
