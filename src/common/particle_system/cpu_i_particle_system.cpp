#include "cpu_i_particle_system.h"
#include "../logger.h"

CpuIParticleSystem::CpuIParticleSystem(uint32_t maxParticles)
	: mNumMaxParticles(maxParticles)
	, mNumParticles(0)
	, mMinLifetime(0.0f)
	, mMaxLifetime(0.0f)
	, mMinStartVelocity(0)
	, mMaxStartVelocity(0)
	, mPosition(0.0f)
	, mScale(1.0f)
{
	mParticles.resize(mNumMaxParticles);
}

CpuIParticleSystem::~CpuIParticleSystem()
{
	for (uint32_t i = 0; i < mModules.size(); i++)
	{
		delete mModules[i];
	}
}

void CpuIParticleSystem::SetMinLifetime(float minLifetime)
{
	mMinLifetime = minLifetime;
}

void CpuIParticleSystem::SetMaxLifetime(float maxLifetime)
{
	mMaxLifetime = maxLifetime;
}

void CpuIParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
	mMinStartVelocity = minVelocity;
}

void CpuIParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
	mMaxStartVelocity = maxVelocity;
}

bool CpuIParticleSystem::AddModule(CpuIModule* psModule)
{
	mModules.emplace_back(psModule);
	return true;
}

uint32_t CpuIParticleSystem::GetCurrentParticles() const
{
	return mNumParticles;
}

uint32_t CpuIParticleSystem::GetMaxParticles() const
{
	return mNumMaxParticles;
}

void CpuIParticleSystem::SetPosition(const glm::vec3& position)
{
	mPosition = position;
}

glm::vec3 CpuIParticleSystem::GetPosition() const
{
	return mPosition;
}

void CpuIParticleSystem::SetScale(float scale)
{
	mScale = scale;
}

float CpuIParticleSystem::GetScale() const
{
	return mScale;
}

void CpuIParticleSystem::SortParticles()
{
	OPTICK_EVENT();
	std::sort(mParticles.begin(), mParticles.end());
}

void CpuIParticleSystem::PrepareRender(Camera* camera)
{
	mQuad1 = glm::normalize(glm::cross(camera->Front, camera->Up));
	mQuad2 = glm::normalize(glm::cross(camera->Front, mQuad1));
}

void CpuIParticleSystem::LateUpdate()
{
}

void CpuIParticleSystem::Emit(uint32_t numToGenerate)
{
	// Remembering last position would partially interfere with other optimizations
	uint32_t generatedParticles = 0;
	uint32_t start =
#if SORT
		mNumParticles;
#else
		0;
#endif
	for (uint32_t i = start; i < mNumMaxParticles && generatedParticles < numToGenerate; i++)
	{
		if (mParticles[i].Active)
		{
			continue;
		}
		InitParticle(mParticles[i], true);
		generatedParticles++;
	}
	mNumParticles += generatedParticles;
}

void CpuIParticleSystem::InitParticles(uint32_t initFrom, bool active)
{
	for (uint32_t i = initFrom; i < mNumMaxParticles; i++)
	{
		InitParticle(mParticles[i], active);
	}
}

void CpuIParticleSystem::InitParticle(Particle& particle, bool active)
{
	particle.Active = active;

	particle.Position = mPosition;

	particle.Velocity.x = mRandom.Rand(mMinStartVelocity.x, mMaxStartVelocity.x);
	particle.Velocity.y = mRandom.Rand(mMinStartVelocity.y, mMaxStartVelocity.y);
	particle.Velocity.z = mRandom.Rand(mMinStartVelocity.z, mMaxStartVelocity.z);

	float lifetime = mRandom.Rand(mMinLifetime, mMaxLifetime);
	particle.Lifetime = lifetime;
	particle.BeginLifetime = lifetime;

	particle.Seed = mRandom.Rand(0, mNumMaxParticles);
}