#include "cpu_module_noise_position.h"

#include "cpu_i_particle_system.h"

#include <algorithm>

constexpr float PI = 3.1415926535897932384626433832795028841971693993751058209f;
constexpr float HALF_PI = PI / 2.0f;
constexpr float THREE_HALFS_PI = HALF_PI * 3.0f;
constexpr float TWO_PI  = PI * 2.0f;
constexpr uint32_t NUM_COMPONENTS = 3;

CpuModuleNoisePosition::CpuModuleNoisePosition(CpuIParticleSystem* particleSystem)
	: CpuModuleNoisePosition(particleSystem, glm::vec3(0.0f), glm::vec3(0.0f))
{
}

CpuModuleNoisePosition::CpuModuleNoisePosition(CpuIParticleSystem* particleSystem, const glm::vec3& minNoise, const glm::vec3& maxNoise)
	: CpuModuleNoisePosition(particleSystem, minNoise, maxNoise, 128, 3, std::mt19937::default_seed)
{
}

CpuModuleNoisePosition::CpuModuleNoisePosition(CpuIParticleSystem* particleSystem, const glm::vec3& minNoise, const glm::vec3& maxNoise, uint32_t resolution, uint32_t waves, uint32_t seed)
	: CpuIModule(particleSystem)
	, mNoiseMap(nullptr)
	, mWaveParams(nullptr)
	, mIndices(nullptr)
	, mMinNoise(minNoise)
	, mMaxNoise(maxNoise)
	, mResolution(resolution)
	, mWaves(waves)
	, mRandom(seed)
{
	mWaveParams = new glm::vec2*[mWaves];
	for (uint32_t i = 0; i < mWaves; i++)
	{
		mWaveParams[i] = new glm::vec2[NUM_COMPONENTS];
		for (uint32_t j = 0; j < NUM_COMPONENTS; j++)
		{
			mWaveParams[i][j] = glm::vec2(mRandom.Rand(0.0f, TWO_PI), mRandom.Rand(TWO_PI / mResolution, 0.1f));
		}
	}

	mIndices = new glm::vec2[particleSystem->GetMaxParticles()];
	for (uint32_t i = 0; i < particleSystem->GetMaxParticles(); i++)
	{
		mIndices[i] = glm::vec2(mRandom.Rand(0.0f, mResolution), std::floor(mRandom.Rand(0.0f, mResolution)));
	}

	glm::vec3 min = glm::vec3(1.0f);
	glm::vec3 max = glm::vec3(0.0f);
	mNoiseMap = new glm::vec3*[mResolution];
	for (uint32_t i = 0; i < mResolution; i++)
	{
		mNoiseMap[i] = new glm::vec3[mResolution];
		for (uint32_t j = 0; j < mResolution; j++)
		{
			mNoiseMap[i][j] = glm::vec3(1.0f);
			for (uint32_t k = 0; k < mWaves; k++)
			{
				mNoiseMap[i][j].x *= CalculateWaveValue(i, j, mWaveParams[k][0]);
				mNoiseMap[i][j].y *= CalculateWaveValue(i, j, mWaveParams[k][1]);
				mNoiseMap[i][j].z *= CalculateWaveValue(i, j, mWaveParams[k][2]);
			}

			min.x = std::min(mNoiseMap[i][j].x, min.x);
			min.y = std::min(mNoiseMap[i][j].y, min.y);
			min.z = std::min(mNoiseMap[i][j].z, min.z);
			max.x = std::max(mNoiseMap[i][j].x, max.x);
			max.y = std::max(mNoiseMap[i][j].y, max.y);
			max.z = std::max(mNoiseMap[i][j].z, max.z);
		}
	}

	for (uint32_t i = 0; i < mResolution; i++)
	{
		for (uint32_t j = 0; j < mResolution; j++)
		{
			mNoiseMap[i][j] =
				glm::vec3(
					InvLerp(min.x, max.x, mNoiseMap[i][j].x),
					InvLerp(min.y, max.y, mNoiseMap[i][j].y),
					InvLerp(min.z, max.z, mNoiseMap[i][j].z)
				);
		}
	}
	for (uint32_t j = 0; j < mResolution; j++)
	{
		printf("%g\n", mNoiseMap[0][j].x);
	}
}

CpuModuleNoisePosition::~CpuModuleNoisePosition()
{
	for (uint32_t i = 0; i < mWaves; i++)
	{
		delete[] mWaveParams[i];
	}
	delete[] mWaveParams;

	for (uint32_t i = 0; i < mResolution; i++)
	{
		delete[] mNoiseMap[i];
	}
	delete[] mNoiseMap;
}

float CpuModuleNoisePosition::CalculateWaveValue(float x, float y, glm::vec2 waveParams)
{
	float paramX = std::sin(waveParams.x + x * waveParams.y);
	float paramY = std::sin(waveParams.x + y * waveParams.y);

	return paramX * paramY;
}

float CpuModuleNoisePosition::Lerp(float min, float max, float t)
{
	return min + t * (max - min);
}

float CpuModuleNoisePosition::InvLerp(float min, float max, float t)
{
	return (t - min) / (max - min);
}

void CpuModuleNoisePosition::PreRun(float deltaTime)
{
}

void CpuModuleNoisePosition::UpdateParticle(float deltaTime, Particle& particle, uint32_t index)
{
	if (!Active | !particle.Active)
		return;

	mIndices[index].x += deltaTime;
	uint32_t x = uint32_t(InvLerp(0.0f, particle.BeginLifetime, particle.Lifetime) * mResolution);
	uint32_t y = uint32_t(mIndices[index].y);

	glm::vec3 positionTs = mNoiseMap[x][y];
	glm::vec3 positionAdd =
		glm::vec3(
			Lerp(mMinNoise.x, mMaxNoise.x, positionTs.x),
			Lerp(mMinNoise.y, mMaxNoise.y, positionTs.y),
			Lerp(mMinNoise.z, mMaxNoise.z, positionTs.z)
		);
	particle.Position += positionAdd * deltaTime;
}