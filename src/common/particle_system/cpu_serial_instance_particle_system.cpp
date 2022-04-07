#include "cpu_serial_instance_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"
#include "cpu_module_emission.h"

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

CpuSerialInstanceParticleSystem::CpuSerialInstanceParticleSystem(uint32_t maxParticles)
	: mNumMaxParticles(maxParticles)
	, mNumParticles(0)
	, mVao(0)
	, mVboParticlePosition(0)
	, mVboParticleData(0)
{
	mParticles.resize(mNumMaxParticles);
	mParticleRenderData.resize(CpuInstanceRenderParticle::ParticleSize * mNumMaxParticles);
}

CpuSerialInstanceParticleSystem::~CpuSerialInstanceParticleSystem()
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

bool CpuSerialInstanceParticleSystem::Init()
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

void CpuSerialInstanceParticleSystem::InitParticles(uint32_t initFrom, bool active)
{
	for (uint32_t i = initFrom; i < mNumMaxParticles; i++)
	{
		InitParticle(mParticles[i], active);
	}
}

void CpuSerialInstanceParticleSystem::InitParticle(Particle& particle, bool active)
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

void CpuSerialInstanceParticleSystem::BuildParticleVertexData()
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

void CpuSerialInstanceParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
	//Emit particles
	for (uint32_t j = 0; j < mModules.size(); j++)
	{
		mModules[j]->PreRun(deltaTime);
	}

	// Update all particles
	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		mParticles[i].CameraDistance = glm::distance2(mParticles[i].Position, cameraPos);

		mParticles[i].Lifetime -= deltaTime;
		if (mParticles[i].Lifetime <= 0.0f
			&& mParticles[i].Active)
		{
			mParticles[i].Lifetime = 0.0f;
			mParticles[i].Active = false;
			mParticles[i].CameraDistance = -1.0f;
			mNumParticles--;
		}

		for (uint32_t j = 0; j < mModules.size(); j++)
		{
			mModules[j]->UpdateParticle(deltaTime, mParticles[i]);
		}
	}

	SortParticles();

	// Write data to array
	BuildParticleVertexData();
}

void CpuSerialInstanceParticleSystem::SortParticles()
{
	std::sort(mParticles.begin(), mParticles.end());
}

void CpuSerialInstanceParticleSystem::SetMinLifetime(float minLifetime)
{
	mMinLifetime = minLifetime;
}

void CpuSerialInstanceParticleSystem::SetMaxLifetime(float maxLifetime)
{
	mMaxLifetime = maxLifetime;
}

void CpuSerialInstanceParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
	mMinStartVelocity = minVelocity;
}

void CpuSerialInstanceParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
	mMaxStartVelocity = maxVelocity;
}

bool CpuSerialInstanceParticleSystem::AddModule(CpuIModule* cpuModule)
{
	mModules.emplace_back(cpuModule);
	return true;
}

uint32_t CpuSerialInstanceParticleSystem::GetCurrentParticles() const
{
	return mNumParticles;
}

void CpuSerialInstanceParticleSystem::Emit(uint32_t numToGenerate)
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

void CpuSerialInstanceParticleSystem::RenderParticles()
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
}