#include "cpu_parallel_instance_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"
#include "cpu_module_emission.h"

#if _WIN32
#include <Windows.h>
#endif

#define TX1 (0.0)
#define TY1 (0.0)
#define TX2 (1.0)
#define TY2 (1.0)

static float sBasePlaneVertexPositions[] =
{
	// Coord			// Tex Coord
	-0.5, -0.5,  0.0,	TX1, TY1,
	 0.5, -0.5,  0.0,	TX2, TY1,
	-0.5,  0.5,  0.0,	TX1, TY2,
	 0.5, -0.5,  0.0,	TX2, TY1,
	-0.5,  0.5,  0.0,	TX1, TY2,
	 0.5,  0.5,  0.0,	TX2, TY2
};

void CpuParallelInstanceParticleSystem::Worker::Init(std::vector<CpuIModule*>* modules, std::vector<Particle>* particles, uint32_t threadIndex)
{
	mModules = modules;
	mParticles = particles;
	mThreadId = threadIndex;
}

void CpuParallelInstanceParticleSystem::Worker::StartUpdateParticles(size_t startIndex, size_t endIndex, const glm::vec3& cameraPos, float deltaTime)
{
	mRemovedParticles = 0;
	mStartIndex = startIndex;
	mEndIndex = endIndex;
	mCameraPos = cameraPos;
	mDeltaTime = deltaTime;
	//LOGE("Thread", "Thread #%d working on: %d to %d", mThreadId, mStartIndex, mEndIndex);
	mWorkerThread = std::thread([this]
		{
			UpdateParticles();
		});

#ifdef _WIN32
	DWORD_PTR dw = SetThreadAffinityMask(mWorkerThread.native_handle(), DWORD_PTR(1) << mThreadId);
	if (dw == 0)
	{
		DWORD dwErr = GetLastError();
		LOGE("ParticleWorker", "SetThreadAffinityMask failed, GLE=%llu)", dwErr);
	}
#endif
}

void CpuParallelInstanceParticleSystem::Worker::UpdateParticles()
{
	std::vector<Particle>& particles = *mParticles;
	std::vector<CpuIModule*>& modules = *mModules;

	for (size_t i = mStartIndex; i < mEndIndex; i++)
	{
		particles[i].CameraDistance = glm::distance2(particles[i].Position, mCameraPos);

		particles[i].Lifetime -= mDeltaTime;
		if (particles[i].Lifetime <= 0.0f
			&& particles[i].Active)
		{
			particles[i].Lifetime = 0.0f;
			particles[i].Active = false;
			particles[i].CameraDistance = -1.0f;
			mRemovedParticles++;
		}

		for (uint32_t j = 0; j < modules.size(); j++)
		{
			modules[j]->UpdateParticle(mDeltaTime, particles[i]);
		}
	}
}

void CpuParallelInstanceParticleSystem::Worker::Join()
{
	mWorkerThread.join();
}

uint32_t CpuParallelInstanceParticleSystem::Worker::GetRemovedParticles() const
{
	return mRemovedParticles;
}

CpuParallelInstanceParticleSystem::CpuParallelInstanceParticleSystem(uint32_t maxParticles, uint32_t threads)
	: mNumMaxParticles(maxParticles)
	, mNumParticles(0)
	, mVao(0)
	, mVboParticlePosition(0)
	, mVboParticleData(0)
{
	mParticles.resize(mNumMaxParticles);
	mParticleRenderData.resize(CpuInstanceRenderParticle::ParticleSize * mNumMaxParticles);

	mWorkers.resize(threads);
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		mWorkers[i].Init(&mModules, &mParticles, i);
	}
}

CpuParallelInstanceParticleSystem::~CpuParallelInstanceParticleSystem()
{
	if (mVao != 0)
	{
		glDeleteVertexArrays(1, &mVao);
	}
	if (mVboParticlePosition != 0)
	{
		glDeleteBuffers(1, &mVboParticlePosition);
	}
	if (mVboParticleData != 0)
	{
		glDeleteBuffers(1, &mVboParticleData);
	}
	for (uint32_t i = 0; i < mModules.size(); i++)
	{
		delete mModules[i];
	}
}

bool CpuParallelInstanceParticleSystem::Init()
{
	glGenVertexArrays(1, &mVao);
	glGenBuffers(1, &mVboParticlePosition);
	glGenBuffers(1, &mVboParticleData);

	glBindBuffer(GL_ARRAY_BUFFER, mVboParticleData);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mParticleRenderData.size(), &mParticleRenderData[0], GL_STREAM_DRAW);

	uint32_t offset = 0;
	glBindVertexArray(mVao);
	glBindBuffer(GL_ARRAY_BUFFER, mVboParticlePosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sBasePlaneVertexPositions), sBasePlaneVertexPositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)offset);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, mVboParticleData);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CpuInstanceRenderParticle::ParticleRealSize, (void*)offset);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, CpuInstanceRenderParticle::ParticleRealSize, (void*)(offset += CpuInstanceRenderParticle::PositionRealSize));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);

	InitParticles(0, false);
	BuildParticleVertexData();
	return true;
}

void CpuParallelInstanceParticleSystem::InitParticles(uint32_t initFrom, bool active)
{
	for (uint32_t i = initFrom; i < mNumMaxParticles; i++)
	{
		InitParticle(mParticles[i], active);
	}
}

void CpuParallelInstanceParticleSystem::InitParticle(Particle& particle, bool active)
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

void CpuParallelInstanceParticleSystem::BuildParticleVertexData()
{
	size_t particlesToDraw = 0;

	for (size_t i = 0; i < mNumMaxParticles; i++)
	{
		size_t particleIndex = particlesToDraw * CpuInstanceRenderParticle::ParticleSize;

		if (mParticles[i].Active)
		{
			//Position
			mParticleRenderData[particleIndex + 0] = mParticles[i].Position.x;
			mParticleRenderData[particleIndex + 1] = mParticles[i].Position.y;
			mParticleRenderData[particleIndex + 2] = mParticles[i].Position.z;

			// Colors
			mParticleRenderData[particleIndex + 0 + CpuInstanceRenderParticle::PositionSize] = mParticles[i].Color.r;
			mParticleRenderData[particleIndex + 1 + CpuInstanceRenderParticle::PositionSize] = mParticles[i].Color.g;
			mParticleRenderData[particleIndex + 2 + CpuInstanceRenderParticle::PositionSize] = mParticles[i].Color.b;
			mParticleRenderData[particleIndex + 3 + CpuInstanceRenderParticle::PositionSize] = mParticles[i].Color.a;

			++particlesToDraw;
			if (particlesToDraw == mNumParticles)
			{
				break;
			}
		}
	}

	if (mNumParticles > 0)
	{
		glBindVertexArray(mVao);
		glBindBuffer(GL_ARRAY_BUFFER, mVboParticleData);
		glBufferSubData(GL_ARRAY_BUFFER, 0, CpuInstanceRenderParticle::ParticleRealSize * mNumParticles, &mParticleRenderData[0]);
	}
}

void CpuParallelInstanceParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
	//Emit particles
	for (uint32_t j = 0; j < mModules.size(); j++)
	{
		mModules[j]->PreRun(deltaTime);
	}

	// Update all particles
	uint32_t particlesPerCore = mNumMaxParticles / mWorkers.size();
	uint32_t assignedParticles = 0;
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		uint32_t particlesToAssign = particlesPerCore;
		if (i == mWorkers.size() - 1
			&& (assignedParticles + particlesToAssign) != mNumMaxParticles)
		{
			particlesToAssign = mNumMaxParticles - assignedParticles;
		}

		mWorkers[i].StartUpdateParticles(assignedParticles, assignedParticles + particlesToAssign, cameraPos, deltaTime);
		assignedParticles += particlesToAssign;
	}
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		mWorkers[i].Join();
		mNumParticles -= mWorkers[i].GetRemovedParticles();
	}

	SortParticles();

	// Write data to array
	BuildParticleVertexData();
}

void CpuParallelInstanceParticleSystem::SortParticles()
{
	std::sort(mParticles.begin(), mParticles.end());
}

void CpuParallelInstanceParticleSystem::SetMinLifetime(float minLifetime)
{
	mMinLifetime = minLifetime;
}

void CpuParallelInstanceParticleSystem::SetMaxLifetime(float maxLifetime)
{
	mMaxLifetime = maxLifetime;
}

void CpuParallelInstanceParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
	mMinStartVelocity = minVelocity;
}

void CpuParallelInstanceParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
	mMaxStartVelocity = maxVelocity;
}

bool CpuParallelInstanceParticleSystem::AddModule(CpuIModule* cpuModule)
{
	mModules.emplace_back(cpuModule);
	return true;
}

uint32_t CpuParallelInstanceParticleSystem::GetCurrentParticles() const
{
	return mNumParticles;
}

void CpuParallelInstanceParticleSystem::Emit(uint32_t numToGenerate)
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

void CpuParallelInstanceParticleSystem::RenderParticles()
{
	if (mNumParticles > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(mVao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, mNumVertices, mNumParticles);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	GlUtil::CheckGlError("Draw");
}