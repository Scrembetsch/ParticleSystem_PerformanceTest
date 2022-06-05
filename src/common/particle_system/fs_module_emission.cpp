#include "fs_module_emission.h"
#include "fs_particle_system.h"

FsModuleEmission::FsModuleEmission(FsParticleSystem* particleSystem)
	: FsModuleEmission(particleSystem, 0.0f)
{
}

FsModuleEmission::FsModuleEmission(FsParticleSystem* particleSystem, float emitRate)
	: FsIModule(particleSystem)
	, EmitRate(emitRate)

{
}

void FsModuleEmission::PreRun(float currentTime, float deltaTime)
{
	float timeForParticle = 1.0f / EmitRate;

	mCurrentGenerateOffset += deltaTime;
	uint32_t numToGenerate = 0;

	numToGenerate = static_cast<uint32_t>(mCurrentGenerateOffset / timeForParticle);
	mCurrentGenerateOffset -= static_cast<float>(numToGenerate) * timeForParticle;

	ParticleSystem->Emit(numToGenerate);
}

std::string FsModuleEmission::GetUniforms() const
{
	return "";
}

std::string FsModuleEmission::GetMethodCall() const
{
	return "";
}

std::string FsModuleEmission::GetModuleMethods() const
{
	return "";
}

void FsModuleEmission::ApplyShaderValues(float currentTime, float deltaTime, Shader* shader)
{
}