#include "cpu_serial_instance_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"
#include "cpu_module_emission.h"

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

CpuSerialInstanceParticleSystem::CpuSerialInstanceParticleSystem(uint32_t maxParticles)
	: CpuIParticleSystem(maxParticles)
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
	CHECK_GL_ERROR();
}

bool CpuSerialInstanceParticleSystem::Init()
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

void CpuSerialInstanceParticleSystem::BuildParticleVertexData()
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

void CpuSerialInstanceParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
	OPTICK_EVENT();

	//Emit particles
	for (uint32_t j = 0; j < mModules.size(); j++)
	{
		mModules[j]->PreRun(deltaTime);
	}

	// Update all particles
	uint32_t updatedParticles = 0;
	for (uint32_t i = 0; i < mNumMaxParticles; i++)
	{
		Particle& particle = mParticles[i];

		particle.Lifetime -= deltaTime * particle.Active;
		if (particle.Lifetime <= 0.0f
			&& particle.Active)
		{
			particle.Lifetime = 0.0f;
			particle.Active = false;
			particle.CameraDistance = -1.0f;
			mNumParticles--;
		}
		if (!particle.Active)
			continue;

		particle.CameraDistance = glm::distance2(particle.Position, cameraPos);

		for (uint32_t j = 0; j < mModules.size(); j++)
		{
			mModules[j]->UpdateParticle(deltaTime, particle, i);
		}
		particle.Position += particle.Velocity * deltaTime;

		updatedParticles++;
		if (updatedParticles == mNumParticles)
		{
			break;
		}
	}

#if SORT
	SortParticles();
#endif

	// Write data to array
	BuildParticleVertexData();
}

void CpuSerialInstanceParticleSystem::RenderParticles()
{
	OPTICK_EVENT();

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
	CHECK_GL_ERROR();
}