#include "cpu_parallel_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"
#include "cpu_module_emission.h"
#include <algorithm>

#if _WIN32
#include <Windows.h>
#undef max
#endif

#define TX1 (0.0)
#define TY1 (0.0)
#define TX2 (1.0)
#define TY2 (1.0)

#if INDEXED
static const CpuRenderParticle sBasePlaneVertexData[] =
{
	// Coord			// Color				// Tex Coord
	CpuRenderParticle{glm::vec3(-0.5, -0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX1, TY1)},
	CpuRenderParticle{glm::vec3(0.5, -0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX2, TY1)},
	CpuRenderParticle{glm::vec3(-0.5,  0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX1, TY2)},
	CpuRenderParticle{glm::vec3(0.5,  0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX2, TY2)}
};
static const uint32_t sPlaneIndices[] =
{ 0, 1, 2, 1, 3, 2 };
#else
static const CpuRenderParticle sBasePlaneVertexData[] =
{
	// Coord			// Color						// Tex Coord
CpuRenderParticle{glm::vec3(-0.5, -0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2(TX1, TY1)},
CpuRenderParticle{glm::vec3(0.5, -0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2(TX2, TY1)},
CpuRenderParticle{glm::vec3(-0.5,  0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2(TX1, TY2)},
CpuRenderParticle{glm::vec3(0.5, -0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2(TX2, TY1)},
CpuRenderParticle{glm::vec3(-0.5,  0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2(TX1, TY2)},
CpuRenderParticle{glm::vec3(0.5,  0.5,  0.0),	glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2(TX2, TY2)}
};
#endif

void CpuParallelParticleSystem::Worker::Run(uint32_t threadId, const glm::vec3& cameraPos, float deltaTime)
{
	mCameraPos = cameraPos;
	mDeltaTime = deltaTime;
	mThreadId = threadId;
	mCurrentJob = NONE;

	mWorkerThread = std::thread([this]
		{
			mRunning = true;
			Run();
		});

#ifdef _WIN32
	DWORD_PTR dw = SetThreadAffinityMask(mWorkerThread.native_handle(), DWORD_PTR(1) << (mThreadId));
	if (dw == 0)
	{
		DWORD dwErr = GetLastError();
		LOGE("ParticleWorker", "SetThreadAffinityMask failed, GLE=%ll)", dwErr);
	}
#endif
}

void CpuParallelParticleSystem::Worker::Run()
{
	while (mRunning)
	{
		switch (mCurrentJob)
		{
		case JOB_UPDATE:
			UpdateParticles();
			break;

		case JOB_SORT:
			Sort();
			break;

		case JOB_BUILD:
			BuildVertices();
			break;

		default:
			continue;
		}

		mJobMutex.lock();
		mJobFinished = true;
		mCurrentJob = NONE;
		mJobMutex.unlock();
	}
}

void CpuParallelParticleSystem::Worker::StartJob(Job job, uint32_t startIndex, uint32_t endIndex)
{
	mJobMutex.lock();
	mCurrentJob = job;
	mStartIndex = startIndex;
	mEndIndex = endIndex;
	mJobMutex.unlock();
}

uint32_t CpuParallelParticleSystem::Worker::Update(std::vector<Particle>& particles, const std::vector<CpuIModule*>& modules, uint32_t startIndex, uint32_t endIndex, const glm::vec3& cameraPos, float deltaTime)
{
	uint32_t removedParticles = 0;

	for (size_t i = startIndex; i < endIndex; i++)
	{
		Particle& particle = particles[i];

		particle.Lifetime -= deltaTime * particle.Active;
		if (particle.Lifetime <= 0.0f
			&& particle.Active)
		{
			particle.Lifetime = 0.0f;
			particle.Active = false;
			particle.CameraDistance = -1.0f;
			removedParticles++;
		}
		if (!particle.Active)
			continue;

		particle.CameraDistance = glm::distance2(particle.Position, cameraPos);

		for (uint32_t j = 0; j < modules.size(); j++)
		{
			modules[j]->UpdateParticle(deltaTime, particle, i);
		}
		particle.Position += particle.Velocity * deltaTime;
	}

	return removedParticles;
}

void CpuParallelParticleSystem::Worker::UpdateParticles()
{
	std::vector<Particle>& particles = mParticleSystem->mParticles;
	std::vector<CpuIModule*>& modules = mParticleSystem->mModules;

	mRemovedParticles = 0;

	mJobMutex.lock();
	uint32_t startIndex = mStartIndex;
	uint32_t endIndex = mEndIndex;
	mJobMutex.unlock();

	mRemovedParticles = Update(particles, modules, startIndex, endIndex, mCameraPos, mDeltaTime);
}

void CpuParallelParticleSystem::Worker::WaitForJob()
{
	do
	{
		std::this_thread::yield();
	}
	while (!mJobFinished);
	mJobFinished = false;
}

void CpuParallelParticleSystem::Worker::Sort()
{
	mJobMutex.lock();
	uint32_t startIndex = mStartIndex;
	uint32_t endIndex = mEndIndex;
	mJobMutex.unlock();

	QuickSort(mParticleSystem, startIndex, endIndex);
}

void CpuParallelParticleSystem::Worker::BuildVertices()
{
	mJobMutex.lock();
	uint32_t startIndex = mStartIndex;
	uint32_t endIndex = mEndIndex;
	mJobMutex.unlock();

	BuildVertices(mParticleSystem, startIndex, endIndex);
}

int32_t CpuParallelParticleSystem::Worker::Partition(std::vector<Particle>& particles, int32_t begin, int32_t end)
{
	Particle& pivot = particles[end];
	int32_t i = begin;

	for (int32_t j = begin; j < end; j++)
	{
		if (particles[j] < pivot)
		{
			std::swap(particles[i], particles[j]);
			i++;
		}
	}
	std::swap(particles[i], particles[end]);
	return i;
}

void CpuParallelParticleSystem::Worker::QuickSort(CpuParallelParticleSystem* ps, int32_t begin, int32_t end)
{
	if (begin < end)
	{
		int32_t pi = Partition(ps->mParticles, begin, end);

		if (ps->mThreadCounter.fetch_add(1) < ps->mWorkers.size())
		{
			if (pi > 0)
			{
				uint32_t threadId = ps->mActiveThreads.fetch_add(1);
#if USE_WORKERS
				ps->mWorkers[threadId]->StartJob(JOB_SORT, begin, pi);
#else
				ps->mWorkers[threadId] = std::thread([ps, begin, pi]()
					{
						QuickSort(ps, begin, pi);
					});
#endif
			}
			QuickSort(ps, pi + 1, end);
			return;
		}
		std::sort(&ps->mParticles[begin], &ps->mParticles[pi]);
		if(end > pi)
			std::sort(&ps->mParticles[pi + 1], &ps->mParticles[end]);
	}
}

void CpuParallelParticleSystem::Worker::BuildVertices(CpuParallelParticleSystem* ps, int32_t begin, int32_t end)
{
#if INDEXED
	const uint32_t verticesPerParticle = mNumIndexedVertices;
#else
	const uint32_t verticesPerParticle = mNumVertices;	// Currently no triangle strip
#endif

	std::vector<Particle>& particles = ps->mParticles;
	std::vector<CpuRenderParticle>& renderData = ps->mParticleRenderData;
	std::atomic_uint32_t& particlesToDrawAtomic = ps->mParticlesToDraw;

	for (size_t i = begin; i < end; i++)
	{
		if (particles[i].Active)
		{
#if SORT
			size_t particleIndex = i * verticesPerParticle;
#else
			uint32_t particlesToDraw = particlesToDrawAtomic.fetch_add(1);
			size_t particleIndex = particlesToDraw * verticesPerParticle;
#endif

			for (uint32_t j = 0; j < verticesPerParticle; j++)
			{
				size_t vertexIndex = j;

				renderData[particleIndex + vertexIndex].Position = particles[i].Position + sBasePlaneVertexData[vertexIndex].Position;
				renderData[particleIndex + vertexIndex].Color = particles[i].Color;
				renderData[particleIndex + vertexIndex].TexCoord = sBasePlaneVertexData[vertexIndex].TexCoord;
			}

#if not SORT
			particlesToDraw += 1;
			if (particlesToDraw >= ps->mNumParticles)
			{
				break;
			}
#endif
		}
#if SORT
		else
		{
			break;
		}
#endif
	}
}

void CpuParallelParticleSystem::Worker::Join()
{
	mRunning = false;
	mWorkerThread.join();
}

uint32_t CpuParallelParticleSystem::Worker::GetRemovedParticles() const
{
	return mRemovedParticles;
}

CpuParallelParticleSystem::CpuParallelParticleSystem(uint32_t maxParticles, uint32_t threads)
	: CpuIParticleSystem(maxParticles)
	, mVao(0)
	, mVeo(0)
	, mVbo(0)
{
	mParticles.resize(mNumMaxParticles);
#if INDEXED
	mParticleRenderData.resize(CpuRenderParticle::ParticleSize * mNumIndexedVertices * mNumMaxParticles);
	mIndices.resize(mNumVertices * mNumMaxParticles);
#else
	mParticleRenderData.resize(mNumVertices * mNumMaxParticles);
#endif
	mWorkers.resize(threads);
#if USE_WORKERS
	for (uint32_t i = 0; i < threads; i++)
	{
		mWorkers[i] = new Worker(this);
	}
#endif
}

CpuParallelParticleSystem::~CpuParallelParticleSystem()
{
	if (mVao != 0)
	{
		glDeleteVertexArrays(1, &mVao);
	}
	if (mVeo != 0)
	{
		glDeleteVertexArrays(1, &mVeo);
	}
	if (mVbo != 0)
	{
		glDeleteBuffers(1, &mVbo);
	}
#if USE_WORKERS
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		delete mWorkers[i];
	}
#endif
	CHECK_GL_ERROR();
}

bool CpuParallelParticleSystem::Init()
{
	glGenVertexArrays(1, &mVao);
	glGenBuffers(1, &mVbo);

#if INDEXED
	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		uint32_t index = i * mNumVertices;
		mIndices[index + 0] = sPlaneIndices[0] + mNumIndexedVertices * i;
		mIndices[index + 1] = sPlaneIndices[1] + mNumIndexedVertices * i;
		mIndices[index + 2] = sPlaneIndices[2] + mNumIndexedVertices * i;
		mIndices[index + 3] = sPlaneIndices[3] + mNumIndexedVertices * i;
		mIndices[index + 4] = sPlaneIndices[4] + mNumIndexedVertices * i;
		mIndices[index + 5] = sPlaneIndices[5] + mNumIndexedVertices * i;
	}
	glGenBuffers(1, &mVeo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVeo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mIndices.size(), &mIndices[0], GL_STATIC_DRAW);
#endif

	size_t offset = 0;
	glBindVertexArray(mVao);
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CpuRenderParticle) * mParticleRenderData.size(), &mParticleRenderData[0], GL_DYNAMIC_DRAW);
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

#if INDEXED
	const uint32_t verticesPerParticle = mNumIndexedVertices;
#else
	const uint32_t verticesPerParticle = mNumVertices;	// Currently no triangle strip
#endif
	size_t particlesToDraw = 0;

	for (size_t i = 0; i < mNumMaxParticles; i++)
	{
		size_t particleIndex = particlesToDraw * verticesPerParticle;

		if (mParticles[i].Active)
		{
			for (uint32_t j = 0; j < verticesPerParticle; j++)
			{
				size_t vertexIndex = j;

				mParticleRenderData[particleIndex + vertexIndex].Position = mParticles[i].Position + sBasePlaneVertexData[vertexIndex].Position;
				mParticleRenderData[particleIndex + vertexIndex].Color = mParticles[i].Color;
				mParticleRenderData[particleIndex + vertexIndex].TexCoord = sBasePlaneVertexData[vertexIndex].TexCoord;
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

#if USE_WORKERS && (PARALLEL_UPDATE || PARALLEL_SORT || PARALLEL_BUILD)
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		mWorkers[i]->Run(i, cameraPos, deltaTime);
	}
#endif
	//Emit particles
	for (uint32_t j = 0; j < mModules.size(); j++)
	{
		mModules[j]->PreRun(deltaTime);
	}

	uint32_t particlesPerCore = mNumMaxParticles / mWorkers.size();
	uint32_t assignedParticles = 0;

#if PARALLEL_UPDATE
	// Update all particles

#if not USE_WORKERS
	std::mutex particleUpdateMutex;
#endif

	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		uint32_t particlesToAssign = particlesPerCore;
		if (i == mWorkers.size() - 1
			&& (assignedParticles + particlesToAssign) != mNumMaxParticles)
		{
			particlesToAssign = mNumMaxParticles - assignedParticles;
		}

#if USE_WORKERS
		mWorkers[i]->StartJob(Worker::JOB_UPDATE, assignedParticles, assignedParticles + particlesToAssign);
#else
		mWorkers[i] = std::thread([this, assignedParticles, particlesToAssign, cameraPos, deltaTime, &particleUpdateMutex]()
			{
				uint32_t removedParticles = Worker::Update(mParticles, mModules, assignedParticles, assignedParticles + particlesToAssign, cameraPos, deltaTime);

				particleUpdateMutex.lock();
				mNumParticles -= removedParticles;
				particleUpdateMutex.unlock();
			});
#endif
		assignedParticles += particlesToAssign;
	}
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
#if USE_WORKERS
		mWorkers[i]->WaitForJob();
		mNumParticles -= mWorkers[i]->GetRemovedParticles();
#else
		mWorkers[i].join();
#endif
	}
#else
	mNumParticles -= Worker::Update(mParticles, mModules, 0, mNumMaxParticles, cameraPos, deltaTime);
#endif


#if SORT
#if PARALLEL_SORT
	mThreadCounter = 1;
	mActiveThreads = 1;

#if USE_WORKERS
	mWorkers[0]->StartJob(Worker::JOB_SORT, 0, mNumMaxParticles - 1);
	for (uint32_t i = 0; i < mActiveThreads; i++)
	{
		mWorkers[i]->WaitForJob();
	}
#else
	mWorkers[0] = std::thread([this]()
	{
		Worker::QuickSort(this, 0, mNumMaxParticles - 1);
	});
	for (uint32_t i = 0; i < mActiveThreads; i++)
	{
		mWorkers[i].join();
	}
	// Thread
#endif
#else
	SortParticles();
#endif
#endif

#if PARALLEL_BUILD
	assignedParticles = 0;
	mParticlesToDraw = 0;
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		uint32_t particlesToAssign = particlesPerCore;
		if (i == mWorkers.size() - 1
			&& (assignedParticles + particlesToAssign) != mNumMaxParticles)
		{
			particlesToAssign = mNumMaxParticles - assignedParticles;
		}

#if USE_WORKERS
		mWorkers[i]->StartJob(Worker::JOB_BUILD, assignedParticles, assignedParticles + particlesToAssign);
#else
		mWorkers[i] = std::thread([this, assignedParticles, particlesToAssign, cameraPos, deltaTime]()
			{
				Worker::BuildVertices(this, assignedParticles, assignedParticles + particlesToAssign);
			});
#endif
		assignedParticles += particlesToAssign;
	}
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
#if USE_WORKERS
		mWorkers[i]->WaitForJob();
#else
		mWorkers[i].join();
#endif
	}
	if (mNumParticles > 0)
	{
#if INDEXED
		const uint32_t verticesPerParticle = mNumIndexedVertices;
#else
		const uint32_t verticesPerParticle = mNumVertices;	// Currently no triangle strip
#endif

		glBindVertexArray(mVao);
		glBufferSubData(GL_ARRAY_BUFFER, 0, CpuRenderParticle::ParticleRealSize * mNumParticles * verticesPerParticle, &mParticleRenderData[0]);
	}
	CHECK_GL_ERROR();
#else
	// Write data to array
	BuildParticleVertexData();
#endif

#if USE_WORKERS && (PARALLEL_UPDATE || PARALLEL_SORT || PARALLEL_BUILD)
	for (uint32_t i = 0; i < mWorkers.size(); i++)
	{
		mWorkers[i]->Join();
	}
#endif
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
#if INDEXED
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVeo);
		glDrawElements(GL_TRIANGLES, mNumParticles * mNumVertices, GL_UNSIGNED_INT, 0);
#else
		glDrawArrays(GL_TRIANGLES, 0, mNumParticles * mNumVertices);
#endif
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	CHECK_GL_ERROR();
}
