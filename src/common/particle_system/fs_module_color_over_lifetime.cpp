#include "fs_module_color_over_lifetime.h"
#include "fs_particle_system.h"

static const char sMethodCall[] = "ColorOverLifeModule();";
static const char sMethod[] = "void ColorOverLifeModule()\n"
"{\n"
"  if(lParticle.Alive)\n"
"  {\n"
"    vec4 color = uColorBegin + (1.0 - lParticle.TimeT) * (uColorEnd - uColorBegin);\n"
"    lParticle.Color = color;\n"
"  }\n"
"}\n";

static const char sUniforms[] =
"uniform vec4 uColorBegin;\n"
"uniform vec4 uColorEnd;\n";

FsModuleColorOverLifetime::FsModuleColorOverLifetime(FsParticleSystem* particleSystem)
	: FsModuleColorOverLifetime(particleSystem, glm::vec4(1.0f), glm::vec4(1.0f))
{
}

FsModuleColorOverLifetime::FsModuleColorOverLifetime(FsParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd)
	: FsIModule(particleSystem)
	, ColorBegin(colorBegin)
	, ColorEnd(colorEnd)

{
}

void FsModuleColorOverLifetime::PreRun(float currentTime, float deltaTime)
{
}

std::string FsModuleColorOverLifetime::GetUniforms() const
{
	return sUniforms;
}

std::string FsModuleColorOverLifetime::GetMethodCall() const
{
	return sMethodCall;
}

std::string FsModuleColorOverLifetime::GetModuleMethods() const
{
	return sMethod;
}

void FsModuleColorOverLifetime::ApplyShaderValues(float currentTime, float deltaTime, Shader* shader)
{
	shader->SetVec4("uColorBegin", ColorBegin);
	shader->SetVec4("uColorEnd", ColorEnd);
}