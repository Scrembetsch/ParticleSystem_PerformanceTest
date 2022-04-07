#include "tf_module_color_over_lifetime.h"

#include "tf_particle_system.h"

static const char sMethodCall[] = "ColorOverLifeModule();";
static const char sMethod[] = "void ColorOverLifeModule()\n"
"{\n"
"  if(vTypeOut != 0.0)\n"
"  {\n"
"    float t = 1.0 - (vLifeTimeOut / vLifeTimeBeginOut);\n"
"    vec4 color = uColorBegin + t * (uColorEnd - uColorBegin);\n"
"    vColorOut = color;\n"
"  }\n"
"}\n";

static const char sUniforms[] =
"uniform vec4 uColorBegin;\n"
"uniform vec4 uColorEnd;\n";

TfModuleColorOverLife::TfModuleColorOverLife(TfParticleSystem* particleSystem)
	: TfModuleColorOverLife(particleSystem, glm::vec4(1.0f), glm::vec4(1.0f))
{
}

TfModuleColorOverLife::TfModuleColorOverLife(TfParticleSystem* particleSystem, const glm::vec4& colorBegin, const glm::vec4& colorEnd)
	: TfIModule(particleSystem)
	, ColorBegin(colorBegin)
	, ColorEnd(colorEnd)
{
}

std::string TfModuleColorOverLife::GetMethodCall(Shader::ShaderType shaderType)
{
	switch (shaderType)
	{
	case Shader::ShaderType::SHADER_TYPE_GEOMETRY:
		return sMethodCall;

	default:
		return "";
	}
}

std::string TfModuleColorOverLife::GetModuleMethods(Shader::ShaderType shaderType)
{
	switch (shaderType)
	{
	case Shader::ShaderType::SHADER_TYPE_GEOMETRY:
		return sMethod;

	default:
		return "";
	}
}

std::string TfModuleColorOverLife::GetUniforms()
{
	return sUniforms;
}

void TfModuleColorOverLife::ApplyUniforms(float deltaTime, Shader* shader)
{
	shader->SetVec4("uColorBegin", ColorBegin);
	shader->SetVec4("uColorEnd", ColorEnd);
}
