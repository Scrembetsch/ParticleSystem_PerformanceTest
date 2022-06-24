#pragma once

#include "cpu_i_module.h"
#include "../util/random.h"

class CpuModuleNoisePosition : public CpuIModule
{
public:
	explicit CpuModuleNoisePosition(CpuIParticleSystem* particleSystem);
	CpuModuleNoisePosition(CpuIParticleSystem* particleSystem, const glm::vec3& minNoise, const glm::vec3& maxNoise);
	CpuModuleNoisePosition(CpuIParticleSystem* particleSystem, const glm::vec3& minNoise, const glm::vec3& maxNoise, uint32_t resolution, uint32_t waves, uint32_t seed);
	~CpuModuleNoisePosition() override;


	void PreRun(float deltaTime) override;
	void UpdateParticle(float deltaTime, Particle& particle, uint32_t index) override;

private:
	static float CalculateWaveValue(float x, float y, glm::vec2 waveParams);
	static float Lerp(float min, float max, float t);
	static float InvLerp(float min, float max, float t);

	glm::vec3** mNoiseMap;
	glm::vec2** mWaveParams;

	glm::vec2* mIndices;

	glm::vec3 mMinNoise;
	glm::vec3 mMaxNoise;

	uint32_t mResolution;
	uint32_t mWaves;

	Random mRandom;
};