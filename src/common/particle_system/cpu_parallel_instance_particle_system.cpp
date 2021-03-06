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

static const float sBasePlaneVertexPositions[] =
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

	mWorkerThread = std::thread([this]
		{
			UpdateParticles();
		});

#ifdef _WIN32
	DWORD_PTR dw = SetThreadAffinityMask(mWorkerThread.native_handle(), DWORD_PTR(1) << mThreadId);
	if (dw == 0)
	{
		DWORD dwErr = GetLastError();
		LOGE("ParticleWorker", "SetThreadAffinityMask failed, GLE=%lu)", dwErr);
	}
#endif
}

void CpuParallelInstanceParticleSystem::Worker::UpdateParticles()
{
	std::vector<Particle>& particles = *mParticles;
	std::vector<CpuIModule*>& modules = *mModules;

	for (size_t i = mStartIndex; i < mEndIndex; i++)
	{
		Particle& particle = particles[i];

		particle.Lifetime -= mDeltaTime * particle.Active;
		if (particle.Lifetime <= 0.0f
			&& particle.Active)
		{
			particle.Lifetime = 0.0f;
			particle.Active = false;
			particle.CameraDistance = -1.0f;
			mRemovedParticles++;
		}
		if (!particle.Active)
			continue;

		for (uint32_t j = 0; j < modules.size(); j++)
		{
			modules[j]->UpdateParticle(mDeltaTime, particle, i);
		}
		particle.Position += particle.Velocity * mDeltaTime;
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
	: CpuIParticleSystem(maxParticles)
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
	CHECK_GL_ERROR();
}

bool CpuParallelInstanceParticleSystem::Init()
{
	glGenVertexArrays(1, &mVao);
	glGenBuffers(1, &mVboParticlePosition);
	glGenBuffers(1, &mVboParticleData);

	glBindBuffer(GL_ARRAY_BUFFER, mVboParticleData);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mParticleRenderData.size(), &mParticleRenderData[0], GL_STREAM_DRAW);

	glBindVertexArray(mVao);
	glBindBuffer(GL_ARRAY_BUFFER, mVboParticlePosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sBasePlaneVertexPositions), sBasePlaneVertexPositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, mVboParticleData);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CpuInstanceRenderParticle::ParticleRealSize, (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, CpuInstanceRenderParticle::ParticleRealSize, (void*)(CpuInstanceRenderParticle::PositionRealSize));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);
	CHECK_GL_ERROR();
	InitParticles(0, false);
	return true;
}

void CpuParallelInstanceParticleSystem::BuildParticleVertexData()
{
	OPTICK_EVENT();

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
	CHECK_GL_ERROR();
}

void CpuParallelInstanceParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
	OPTICK_EVENT();
	//Emit particles
	for (uint32_t j = 0; j < mModules.size(); j++)
	{
		mModules[j]->PreRun(deltaTime);
	}

	// Update all particles
	uint32_t workerSize = static_cast<uint32_t>(mWorkers.size());
	uint32_t particlesPerCore = mNumMaxParticles / workerSize;
	uint32_t assignedParticles = 0;
	for (uint32_t i = 0; i < workerSize; i++)
	{
		uint32_t particlesToAssign = particlesPerCore;
		if (i == workerSize - 1
			&& (assignedParticles + particlesToAssign) != mNumMaxParticles)
		{
			particlesToAssign = mNumMaxParticles - assignedParticles;
		}

		mWorkers[i].StartUpdateParticles(assignedParticles, assignedParticles + particlesToAssign, cameraPos, deltaTime);
		assignedParticles += particlesToAssign;
	}
	for (uint32_t i = 0; i < workerSize; i++)
	{
		mWorkers[i].Join();
		mNumParticles -= mWorkers[i].GetRemovedParticles();
	}

#if SORT
	SortParticles();
#endif

	// Write data to array
	BuildParticleVertexData();
}

void CpuParallelInstanceParticleSystem::RenderParticles()
{
	OPTICK_EVENT();

	if (mNumParticles > 0)
	{
		glBindVertexArray(mVao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, mNumVertices, mNumParticles);
		glBindVertexArray(0);
	}
	CHECK_GL_ERROR();
}