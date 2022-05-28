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

static const float sBasePlaneVertexData[] =
{
	// Coord			// Color				// Tex Coord
	-0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY1,
	 0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY1,
	-0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY2,
	 0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY1,
	-0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY2,
	 0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY2
};

void CpuParallelParticleSystem::Worker::Init(std::vector<CpuIModule*>* modules, std::vector<Particle>* particles, uint32_t threadIndex, CpuParallelParticleSystem* particleSystem)
{
	mModules = modules;
	mParticles = particles;
	mThreadId = threadIndex;
	mParticleSystem = particleSystem;
	mParticleRenderData = &particleSystem->mParticleRenderData;
}

void CpuParallelParticleSystem::Worker::StartUpdateParticles(size_t startIndex, size_t endIndex, const glm::vec3& cameraPos, float deltaTime)
{
	mRemovedParticles = 0;
	mStartIndex = startIndex;
	mEndIndex = endIndex;
	mCameraPos = cameraPos;
	mDeltaTime = deltaTime;

	mWorkerThread = std::thread( [this]
		{
			UpdateParticles();
		});

#ifdef _WIN32
	DWORD_PTR dw = SetThreadAffinityMask(mWorkerThread.native_handle(), DWORD_PTR(1) << mThreadId);
	if (dw == 0)
	{
		DWORD dwErr = GetLastError();
		LOGE("ParticleWorker", "SetThreadAffinityMask failed, GLE=%ll)", dwErr);
	}
#endif
}

void CpuParallelParticleSystem::Worker::StartVertexBuild(size_t startIndex, size_t endIndex)
{
	mStartIndex = startIndex;
	mEndIndex = endIndex;

	mWorkerThread = std::thread([this]
		{
			BuildVertices();
		});
}

void CpuParallelParticleSystem::Worker::UpdateParticles()
{
	std::vector<Particle>& particles = *mParticles;
	std::vector<CpuIModule*>& modules= *mModules;

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
			modules[j]->UpdateParticle(mDeltaTime, particle);
		}
		particle.Position += particle.Velocity * mDeltaTime;
	}
}

void CpuParallelParticleSystem::Worker::BuildVertices()
{
	const uint32_t verticesPerParticle = mNumVertices;	// Currently no triangle strip
	std::vector<Particle>& particles = *mParticles;
	std::vector<float>& renderData = *mParticleRenderData;
	std::atomic_uint32_t& particlesToDrawAtomic = mParticleSystem->mParticlesToDraw;

	for (size_t i = mStartIndex; i < mEndIndex; i++)
	{
		if (particles[i].Active)
		{
			uint32_t particlesToDraw = particlesToDrawAtomic.fetch_add(1);
			size_t particleIndex = particlesToDraw * CpuRenderParticle::ParticleSize * verticesPerParticle;

			for (uint32_t j = 0; j < verticesPerParticle; j++)
			{
				size_t vertexIndex = j * CpuRenderParticle::ParticleSize;

				//Position
				renderData[particleIndex + vertexIndex + 0] = particles[i].Position.x + sBasePlaneVertexData[vertexIndex + 0];
				renderData[particleIndex + vertexIndex + 1] = particles[i].Position.y + sBasePlaneVertexData[vertexIndex + 1];
				renderData[particleIndex + vertexIndex + 2] = particles[i].Position.z + sBasePlaneVertexData[vertexIndex + 2];

				// Colors
				renderData[particleIndex + vertexIndex + 0 + CpuRenderParticle::PositionSize] = particles[i].Color.r;
				renderData[particleIndex + vertexIndex + 1 + CpuRenderParticle::PositionSize] = particles[i].Color.g;
				renderData[particleIndex + vertexIndex + 2 + CpuRenderParticle::PositionSize] = particles[i].Color.b;
				renderData[particleIndex + vertexIndex + 3 + CpuRenderParticle::PositionSize] = particles[i].Color.a;

				// TexCoord
				renderData[particleIndex + vertexIndex + 0 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize] = sBasePlaneVertexData[vertexIndex + 0 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize];
				renderData[particleIndex + vertexIndex + 1 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize] = sBasePlaneVertexData[vertexIndex + 1 + CpuRenderParticle::PositionSize + CpuRenderParticle::ColorSize];
			}

			if (particlesToDraw >= mParticleSystem->mNumParticles)
			{
				break;
			}
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
		mWorkers[i].Init(&mModules, &mParticles, i, this);
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
	CHECK_GL_ERROR();
}

bool CpuParallelParticleSystem::Init()
{
	glGenVertexArrays(1, &mVao);
	glGenBuffers(1, &mVbo);

	size_t offset = 0;
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

	CHECK_GL_ERROR();
	InitParticles(0, false);
	return true;
}

void CpuParallelParticleSystem::BuildParticleVertexData()
{
	OPTICK_EVENT();

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
	CHECK_GL_ERROR();
}

void CpuParallelParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
	OPTICK_EVENT();

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

#if SORT
	SortParticles();
#endif

	// Write data to array
	BuildParticleVertexData();
}

void CpuParallelParticleSystem::RenderParticles()
{
	OPTICK_EVENT();

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
	CHECK_GL_ERROR();
}

//Particle* CpuParallelParticleSystem::Partition(Particle* begin, Particle* end)
//{
//	//Particle* pivot = mRandom.RandIntegral(begin, end);
//	Particle* pivot = end;
//	Particle* i = begin;
//
//	for (Particle* j = begin; j < end; j++)
//	{
//		if ((*j) < (*pivot))
//		{
//			std::swap(*i, *j);
//			i++;
//		}
//	}
//	std::swap(*i, *end);
//	return i;
//}
//
//void CpuParallelParticleSystem::QuickSort(Particle* begin, Particle* end)
//{
//	if (begin < end)
//	{
//		Particle* pi = Partition(begin, end);
//		if (mSortThreadCounter < mWorkers.size())
//		{
//			uint32_t id = mSortThreadCounter.fetch_add(1);
//			if (id < mWorkers.size())
//			{
//				mWorkers[id].StartSort(begin, pi - 1);
//				QuickSort(pi + 1, end);
//				return;
//			}
//		}
//		QuickSort(begin, pi - 1);
//		QuickSort(pi + 1, end);
//	}
//}
