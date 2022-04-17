#include "tf_module_emission.h"

#include "tf_particle_system.h"

static const std::string sMethodCall = { "EmissionModule();\n" };
static const std::string sMethod = {
"void EmissionModule()\n"
"{\n"
"  if (vTypeOut == 0.0)\n"
"  {\n"
"    EmitVertex();\n"
"    EndPrimitive();\n"
"    for (int i = 0; i < uNumToGenerate && i < MAX_OUTPUT_VERTICES; i++)\n"
"    {\n"
"      InitParticle();\n"
"    }\n"
"  }\n"
"}\n"
};

static const char sUniforms[] = "uniform int uNumToGenerate;\n";

TfModuleEmission::TfModuleEmission(TfParticleSystem* particleSystem)
	: TfModuleEmission(particleSystem, 0.0f)
{
}

TfModuleEmission::TfModuleEmission(TfParticleSystem* particleSystem, float emitRate)
	: TfIModule(particleSystem)
	, EmitRate(emitRate)
	, mCurrentGenerateOffset(0.0f)
{
}

std::string TfModuleEmission::GetMethodCall(Shader::ShaderType shaderType)
{
	switch (shaderType)
	{
	case Shader::ShaderType::SHADER_TYPE_GEOMETRY:
		return sMethodCall;

	default:
		return "";
	}
}

std::string TfModuleEmission::GetModuleMethods(Shader::ShaderType shaderType)
{
	switch (shaderType)
	{
	case Shader::ShaderType::SHADER_TYPE_GEOMETRY:
		return sMethod;

	default:
		return "";
	}
}

std::string TfModuleEmission::GetUniforms()
{
	return sUniforms;
}

void TfModuleEmission::ApplyUniforms(float deltaTime, Shader* shader)
{
	float timeForOneCpuParticle = 1.0f / EmitRate;

	mCurrentGenerateOffset += deltaTime;
	uint32_t numToGenerate = 0;

	// Todo: Could improve
	while (mCurrentGenerateOffset >= timeForOneCpuParticle)
	{
		numToGenerate++;
		mCurrentGenerateOffset -= timeForOneCpuParticle;
	}

	shader->SetInt("uNumToGenerate", numToGenerate);
}
