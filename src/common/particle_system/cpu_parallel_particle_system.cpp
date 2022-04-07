#include "cpu_parallel_particle_system.h"
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

static float sBasePlaneVertexData[] =
{
	// Coord			// Color				// Tex Coord
	-0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY1,
	 0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY1,
	-0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY2,
	 0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY1,
	-0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY2,
	 0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY2
};

void CpuParallelParticleSystem::Worker::Init(std::vector<CpuIModule*>* modules, std::vector<Particle>* particles, uint32_t threadIndex)
{
	mModules = modules;
	mParticles = particles;
	mThreadId = threadIndex;
}

void CpuParallelParticleSystem::Worker::StartUpdateParticles(size_t startIndex, size_t endIndex, const glm::vec3& cameraPos, float deltaTime)
{
	mRemovedParticles = 0;
	mStartIndex = startIndex;
	mEndIndex = endIndex;
	mCameraPos = cameraPos;
	mDeltaTime = deltaTime;
	//LOGE("Thread", "Thread #%d working on: %d to %d", mThreadId, mStartIndex, mEndIndex);
	mWorkerThread = std::thread( [this]
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

void CpuParallelParticleSystem::Worker::UpdateParticles()
{
	std::vector<Particle>& particles = *mParticles;
	std::vector<CpuIModule*>& modules= *mModules;

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

void CpuParallelParticleSystem::Worker::Join()
{
	mWorkerThread.join();
}

uint32_t CpuParallelParticleSystem::Worker::GetRemovedParticles() const
{
	return mRemovedParticles;
}

CpuParallelParticleSystem::CpuParallelParticleSystem(uint32_t maxParticles, uint32_t threads)
	: CpuIParticleSystem(maxParticles)
	, mVao(0)
	, mVbo(0)
{
	mParticles.resize(mNumMaxParticles);
	mParticleRenderData.resize(CpuRenderParticle::ParticleSize * mNumVertices * mNumMaxParticles);

	mWorkers.resize(threads);
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		mWorkers[i].Init(&mModules, &mParticles, i);
	}
}

CpuParallelParticleSystem::~CpuParallelParticleSystem()
{
	if (mVao != 0)
	{
		glDeleteVertexArrays(1, &mVao);
	}
	if (mVbo != 0)
	{
		glDeleteBuffers(1, &mVbo);
	}
}

bool CpuParallelParticleSystem::Init()
{
	glGenVertexArrays(1, &mVao);
	glGenBuffers(1, &mVbo);

	uint32_t offset = 0;
	glBindVertexArray(mVao);
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mParticleRenderData.size(), &mParticleRenderData[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CpuRenderParticle::ParticleRealSize, (void*)offset);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, CpuRenderParticle::ParticleRealSize, (void*)(offset += CpuRenderParticle::PositionRealSize));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, CpuRenderParticle::ParticleRealSize, (void*)(offset += CpuRenderParticle::ColorRealSize));

	glBindVertexArray(0);

	InitParticles(0, false);
	return true;
}

void CpuParallelParticleSystem::BuildParticleVertexData()
{
	const uint32_t verticesPerParticle = mNumVertices;	// Currently no triangle strip

	size_t particlesToDraw = 0;

	for (size_t i = 0; i < mNumMaxParticles; i++)
	{
		size_t particleIndex = particlesToDraw * CpuRenderParticle::ParticleSize * verticesPerParticle;

		if (mParticles[i].Active)
		{
			for (uint32_t j = 0; j < verticesPerParticle; j++)
			{
				size_t vertexIndex = j * CpuRenderParticle::ParticleSize;

				//Position
				mParticleRenderData[particleIndex + vertexIndex + 0] = mParticles[i].Position.x + sBasePlaneVertexData[vertexIndex + 0];
				mParticleRenderData[particleIndex + vertexIndex + 1] = mParticles[i].Position.y + sBasePlaneVertexData[vertexIndex + 1];
				mParticleRenderData[particleIndex + vertexIndex + 2] = mParticles[i].Position.z + sBasePlaneVertexData[vertexIndex + 2];

				// Colors
				mParticleRenderData[particleIndex + vertexIndex + 0 + CpuRenderParticle::PositionSize] = mParticles[i].Color.r;
				mParticleRenderData[particleIndex + vertexIndex + 1 + CpuRenderParticle::PositionSize] = mParticles[i].Color.g;
				mParticleRenderData[particleIndex + vertexIndex + 2 + CpuRenderParticle::PositionSize] = mParticles[i].Color.b;
				mParticleRenderData[particleIndex + vertexIndex + 3 + CpuRenderParticle::PositionSize] = mParticles[i].Color.a;

				// TexCoord
				mParticleRenderData[particleIndex + vertexIndex + 0 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize] = sBasePlaneVertexData[vertexIndex + 0 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize];
				mParticleRenderData[particleIndex + vertexIndex + 1 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize] = sBasePlaneVertexData[vertexIndex + 1 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize];
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
		glBindVertexArray(mVao);
		glBufferSubData(GL_ARRAY_BUFFER, 0, CpuRenderParticle::ParticleRealSize * mNumParticles * verticesPerParticle, &mParticleRenderData[0]);
	}
}

void CpuParallelParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
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

void CpuParallelParticleSystem::RenderParticles()
{
	if (mNumParticles > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(mVao);
		glDrawArrays(GL_TRIANGLES, 0, mNumParticles * mNumVertices);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	CHECK_GL_ERROR("Draw");
}