#include "tf_module_emission.h"

#include "tf_particle_system.h"

static const std::string sMethodCall = { "EmissionModule();\n" };
static const std::string sMethod = {
"void EmissionModule()\n"
"{\n"
"  if (vDataOut.z > 0.0)\n"
"  {\n"
"    EmitVertex();\n"
"    EndPrimitive();\n"

"    float numToEmit = uEmitParams.x;\n"
"    numToEmit += float(vDataOut.z < uEmitParams.y);\n"

"    float maxVertices = float(MAX_OUTPUT_VERTICES);\n"
"    for (float i = 0.0; i < numToEmit && i < maxVertices; i++)\n"
"    {\n"
"      InitParticle();\n"
"    }\n"
"  }\n"
"}\n"
};

static const char sUniforms[] = "uniform vec2 uEmitParams;\n";

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

	numToGenerate = static_cast<uint32_t>(mCurrentGenerateOffset / timeForOneCpuParticle);
	mCurrentGenerateOffset -= static_cast<float>(numToGenerate) * timeForOneCpuParticle;

	glm::vec2 emitParams(0);
	if (numToGenerate == 0)
	{
		// Just to jump over this case
	}
	else if (numToGenerate <= ParticleSystem->GetMaxVerticesPerEmitter())
	{
		emitParams.x = 0;
		emitParams.y = numToGenerate + 1;
	}
	else
	{
		emitParams.x = numToGenerate / ParticleSystem->GetEmitters();
		emitParams.y = numToGenerate - (emitParams.x * ParticleSystem->GetEmitters());

		emitParams.y += 1;
	}

	shader->SetVec2("uEmitParams", emitParams);
}
