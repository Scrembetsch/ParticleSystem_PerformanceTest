#include "cpu_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"
#include "cpu_module_emission.h"

#define TX1 (0.0)
#define TY1 (0.0)
#define TX2 (1.0)
#define TY2 (1.0)

static float sBasePlaneVertexData[] =
{
	// Coord				// Color
	-0.5, -0.5,  0.0,		1.0, 1.0, 1.0, 1.0,
	 0.5, -0.5,  0.0,		1.0, 1.0, 1.0, 1.0,
	-0.5,  0.5,  0.0,		1.0, 1.0, 1.0, 1.0,
	 0.5, -0.5,  0.0,		1.0, 1.0, 1.0, 1.0,
	-0.5,  0.5,  0.0,		1.0, 1.0, 1.0, 1.0,
	 0.5,  0.5,  0.0,		1.0, 1.0, 1.0, 1.0
};

uint32_t numVertices = 6;

CpuParticleSystem::ParticleThread::ParticleThread()
{
}

CpuParticleSystem::ParticleThread::~ParticleThread()
{
	mThread.join();
}

void CpuParticleSystem::ParticleThread::SetData(Particle* start, uint32_t numParticles, float deltaTime)
{
	mStartParticle = start;
	mNumParticles = numParticles;
	mDeltaTime = deltaTime;
}

uint32_t CpuParticleSystem::ParticleThread::GetRemovedParticleCount() const
{
	return mRemovedParticles;
}


bool CpuParticleSystem::ParticleThread::Run()
{
	mRemovedParticles = 0;
	for (uint32_t i = 0; i < mNumParticles; i++)
	{
		Particle* currentParticle = mStartParticle + i;

		if (!currentParticle->Active)
		{
			continue;
		}
		currentParticle->Lifetime -= mDeltaTime;
		if (currentParticle->Lifetime <= 0.0)
		{
			currentParticle->Active = false;
			mRemovedParticles++;
			continue;
		}

		//for (UInt32 j = 0; j < mNumModules; j++)
		//{
		//	mModules[j]->UpdateParticle(*currentParticle, mDeltaTime);
		//}
	}
	return true;
}

CpuParticleSystem::CpuParticleSystem()
	: mNumMaxParticles(50000)
	, mNumParticles(0)
	, mCurrentBuffer(0)
{
	mParticles.resize(mNumMaxParticles);
	mParticleRenderData.resize(sizeof(float) * 7 * numVertices * mNumMaxParticles);

	mModules.push_back(new CpuModuleEmission(this, 10000));
}

CpuParticleSystem::~CpuParticleSystem()
{
	if (mVao != 0)
	{
		glDeleteVertexArrays(sBufferSize, mVao);
	}
	if (mVbo != 0)
	{
		glDeleteBuffers(sBufferSize, mVbo);
	}
	for (uint32_t i = 0; i < mModules.size(); i++)
	{
		delete mModules[i];
	}
}

bool CpuParticleSystem::Init()
{
	glGenVertexArrays(sBufferSize, mVao);
	glGenBuffers(sBufferSize, mVbo);

	for (uint32_t i = 0; i < sBufferSize; i++)
	{
		uint32_t offset = 0;
		glBindVertexArray(mVao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, mVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mParticleRenderData.size(), &mParticleRenderData[0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec4), (void*)offset);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec4), (void*)(offset += sizeof(glm::vec3)));

		glBindVertexArray(0);
	}

	InitParticles(0, false);
	BuildParticleVertexData();
	return true;
}

void CpuParticleSystem::InitParticles(uint32_t initFrom, bool active)
{
	for (uint32_t i = initFrom; i < mNumMaxParticles; i++)
	{
		InitParticle(mParticles[i], active);
	}
}

void CpuParticleSystem::InitParticle(Particle& particle, bool active)
{
	particle.Active = active;

	particle.Position.x = 0.0f;
	particle.Position.y = 0.0f;
	particle.Position.z = 0.0f;

	particle.Velocity.x = mRandom.Rand(mMinStartVelocity.x, mMaxStartVelocity.x);
	particle.Velocity.y = mRandom.Rand(mMinStartVelocity.y, mMaxStartVelocity.y);
	particle.Velocity.z = mRandom.Rand(mMinStartVelocity.z, mMaxStartVelocity.z);

	float lifetime = mRandom.Rand(mMinLifetime, mMaxLifetime);
	particle.Lifetime = lifetime;
	particle.BeginLifetime = lifetime;
}

void CpuParticleSystem::BuildParticleVertexData()
{
	const uint32_t posSize = 3;
	const uint32_t colorSize = 4;

	const uint32_t vertexDataPerParticle = posSize + colorSize;
	const uint32_t verticesPerParticle = numVertices;	// Currently no triangle strip

	size_t particlesToDraw = 0;

	for (size_t i = 0; i < mNumMaxParticles; i++)
	{
		size_t particleIndex = particlesToDraw * vertexDataPerParticle * verticesPerParticle;

		if (mParticles[i].Active)
		{
			for (uint32_t j = 0; j < verticesPerParticle; j++)
			{
				size_t vertexIndex = j * vertexDataPerParticle;

				//Position
				mParticleRenderData[particleIndex + vertexIndex + 0] = mParticles[i].Position.x + sBasePlaneVertexData[vertexIndex + 0];
				mParticleRenderData[particleIndex + vertexIndex + 1] = mParticles[i].Position.y + sBasePlaneVertexData[vertexIndex + 1];
				mParticleRenderData[particleIndex + vertexIndex + 2] = mParticles[i].Position.z + sBasePlaneVertexData[vertexIndex + 2];

				// Colors
				mParticleRenderData[particleIndex + vertexIndex + 0 + posSize] = mParticles[i].Color.r * sBasePlaneVertexData[vertexIndex + 0 + posSize];
				mParticleRenderData[particleIndex + vertexIndex + 1 + posSize] = mParticles[i].Color.g * sBasePlaneVertexData[vertexIndex + 1 + posSize];
				mParticleRenderData[particleIndex + vertexIndex + 2 + posSize] = mParticles[i].Color.b * sBasePlaneVertexData[vertexIndex + 2 + posSize];
				mParticleRenderData[particleIndex + vertexIndex + 3 + posSize] = mParticles[i].Color.a * sBasePlaneVertexData[vertexIndex + 3 + posSize];
			}

			++particlesToDraw;
			if (particlesToDraw == mNumParticles)
			{
				break;
			}
		}
	}

	if (mNumParticles > 0)
	{
		mCurrentBuffer = (mCurrentBuffer + 1) % sBufferSize;
		glBindVertexArray(mVao[mCurrentBuffer]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * mNumParticles * vertexDataPerParticle * verticesPerParticle, &mParticleRenderData[0]);
	}
}

void CpuParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
	//Emit particles
	for (uint32_t j = 0; j < mModules.size(); j++)
	{
		mModules[j]->PreRun(deltaTime);
	}

	// Update all particles
	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		mParticles[i].Lifetime -= deltaTime;
		if (mParticles[i].Lifetime <= 0.0f
			&& mParticles[i].Active)
		{
			mParticles[i].Lifetime = 0.0f;
			mParticles[i].Active = false;
			mNumParticles--;
		}
		mParticles[i].CameraDistance = glm::distance2(mParticles[i].Position, cameraPos);

		for (uint32_t j = 0; j < mModules.size(); j++)
		{
			mModules[j]->UpdateParticle(deltaTime, mParticles[i]);
		}
	}

	SortParticles();

	// Write data to array
	BuildParticleVertexData();
}

void CpuParticleSystem::SortParticles()
{
	std::sort(mParticles.begin(), mParticles.end());
}

void CpuParticleSystem::SetMinLifetime(float minLifetime)
{
	mMinLifetime = minLifetime;
}

void CpuParticleSystem::SetMaxLifetime(float maxLifetime)
{
	mMaxLifetime = maxLifetime;
}

void CpuParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
	mMinStartVelocity = minVelocity;
}

void CpuParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
	mMaxStartVelocity = maxVelocity;
}

bool CpuParticleSystem::AddModule(CpuIModule* cpuModule)
{
	if (dynamic_cast<CpuModuleEmission*>(cpuModule) != nullptr)
	{
		return false;
	}
	else
	{
		mModules.push_back(cpuModule);
	}
}

CpuModuleEmission* CpuParticleSystem::GetEmissionModule()
{
	return dynamic_cast<CpuModuleEmission*>(mModules[0]);
}

uint32_t CpuParticleSystem::GetCurrentParticles() const
{
	return mNumParticles;
}

void CpuParticleSystem::Emit(uint32_t numToGenerate)
{
	// Todo can be improved
	uint32_t generatedParticles = 0;
	for (uint32_t i = 0; i < mNumMaxParticles && generatedParticles < numToGenerate; i++)
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

void CpuParticleSystem::RenderParticles()
{
	if (mNumParticles > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(mVao[mCurrentBuffer]);
		glDrawArrays(GL_TRIANGLES, 0, mNumParticles * numVertices);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
}