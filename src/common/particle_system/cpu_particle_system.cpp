#include "cpu_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"

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

	InitParticles();
	BuildParticleVertexData();
	return true;
}

void CpuParticleSystem::InitParticles()
{
	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		mParticles[i].Active = true;
		mNumParticles++;

		mParticles[i].Position.x = 0.0;
		mParticles[i].Position.y = 0.0;
		mParticles[i].Position.z = 0.0;

		mParticles[i].Velocity.x = mRandom.Rand(-2, 2);
		mParticles[i].Velocity.y = mRandom.Rand(-2, 2);
		mParticles[i].Velocity.z = mRandom.Rand(-2, 2);

		mParticles[i].Lifetime = mRandom.Rand(2, 5);
	}
}

void CpuParticleSystem::BuildParticleVertexData()
{
	const uint32_t posSize = 3;
	const uint32_t colorSize = 4;

	const uint32_t vertexDataPerParticle = posSize + colorSize;
	const uint32_t verticesPerParticle = numVertices;	// Currently no triangle strip

	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		uint32_t particleIndex = i * vertexDataPerParticle * verticesPerParticle;

		for (uint32_t j = 0; j < verticesPerParticle; j++)
		{
			uint32_t vertexIndex = j * vertexDataPerParticle;

			if (mParticles[i].Active)
			{
				//Position
				mParticleRenderData[particleIndex + vertexIndex + 0] = mParticles[i].Position.x + sBasePlaneVertexData[vertexIndex + 0];
				mParticleRenderData[particleIndex + vertexIndex + 1] = mParticles[i].Position.y + sBasePlaneVertexData[vertexIndex + 1];
				mParticleRenderData[particleIndex + vertexIndex + 2] = mParticles[i].Position.z + sBasePlaneVertexData[vertexIndex + 2];

				// Colors
				mParticleRenderData[particleIndex + vertexIndex + 0 + posSize] = sBasePlaneVertexData[vertexIndex + 0 + posSize];
				mParticleRenderData[particleIndex + vertexIndex + 1 + posSize] = sBasePlaneVertexData[vertexIndex + 1 + posSize];
				mParticleRenderData[particleIndex + vertexIndex + 2 + posSize] = sBasePlaneVertexData[vertexIndex + 2 + posSize];
				mParticleRenderData[particleIndex + vertexIndex + 3 + posSize] = sBasePlaneVertexData[vertexIndex + 3 + posSize];
			}
			else
			{
				//Position
				mParticleRenderData[particleIndex + vertexIndex + 0] = 0;
				mParticleRenderData[particleIndex + vertexIndex + 1] = 0;
				mParticleRenderData[particleIndex + vertexIndex + 2] = 0;

				// Colors
				mParticleRenderData[particleIndex + vertexIndex + 0 + posSize] = 0;
				mParticleRenderData[particleIndex + vertexIndex + 1 + posSize] = 0;
				mParticleRenderData[particleIndex + vertexIndex + 2 + posSize] = 0;
				mParticleRenderData[particleIndex + vertexIndex + 3 + posSize] = 0;
			}
		}
	}

	mCurrentBuffer = (mCurrentBuffer + 1) % sBufferSize;
	glBindVertexArray(mVao[mCurrentBuffer]);
	//glBindBuffer(GL_ARRAY_BUFFER, mVbo);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mParticleRenderData.size(), &mParticleRenderData[0], GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * mParticleRenderData.size(), &mParticleRenderData[0]);
}

void CpuParticleSystem::UpdateParticles(float deltaTime)
{
	// Update all particles
	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		mParticles[i].Position += mParticles[i].Velocity * deltaTime;
	}
	// Write data to array
	BuildParticleVertexData();
}

void CpuParticleSystem::RenderParticles()
{
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	glBindVertexArray(mVao[mCurrentBuffer]);
	glDrawArrays(GL_TRIANGLES, 0, mNumMaxParticles);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}