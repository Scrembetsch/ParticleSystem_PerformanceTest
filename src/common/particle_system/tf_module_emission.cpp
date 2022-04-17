#include "tf_module_emission.h"

#include "tf_particle_system.h"

static const std::string sMethodCall = { "EmissionModule();\n" };
static const std::string sMethod = {
"void EmissionModule()\n"
"{\n"
"  if (vTypeOut != 0.0)\n"
"  {\n"
"    EmitVertex();\n"
"    EndPrimitive();\n"
"    float numToEmit = 0.0;\n"
"    if(vTypeOut < uEmitParams.y)\n"
"    {\n"
"        numToEmit = uEmitParams.x;\n"
"    }\n"
"    else if(vTypeOut == uEmitParams.y)\n"
"    {\n"
"        numToEmit = uEmitParams.z;\n"
"    }\n"
"    for (float i = 0; i < numToEmit && i < MAX_OUTPUT_VERTICES; i++)\n"
"    {\n"
"      InitParticle();\n"
"    }\n"
"  }\n"
"}\n"
};

static const char sUniforms[] = "uniform vec3 uEmitParams;\n";

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

	glm::vec3 emitParams(0);
	if (numToGenerate == 0)
	{
		// Just to jump over this case
	}
	else if (numToGenerate <= ParticleSystem->GetMaxVerticesPerEmitter())
	{
		emitParams.x = 0;
		emitParams.y = 1;
		emitParams.z = numToGenerate;
	}
	else
	{
		emitParams.x = ParticleSystem->GetMaxVerticesPerEmitter();
		for (uint32_t i = 0; i < ParticleSystem->GetEmitters(); i++)
		{
			if (numToGenerate < ParticleSystem->GetMaxVerticesPerEmitter())
			{
				emitParams.y = (i + 1);
				emitParams.z = numToGenerate;
				break;
			}
			numToGenerate -= emitParams.x;
		}
	}

	shader->SetVec3("uEmitParams", emitParams);
}
