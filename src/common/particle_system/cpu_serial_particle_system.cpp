#include "cpu_serial_particle_system.h"
#include "../gl/gl.h"
#include "../gl/gl_util.h"
#include "../logger.h"
#include "cpu_module_emission.h"
#include "../defines.h"

#define TX1 (0.0)
#define TY1 (0.0)
#define TX2 (1.0)
#define TY2 (1.0)

#if INDEXED
static const float sBasePlaneVertexData[] =
{
	// Coord			// Color				// Tex Coord
	-0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY1,
	 0.5, -0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY1,
	-0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX1, TY2,
	 0.5,  0.5,  0.0,	1.0, 1.0, 1.0, 1.0,		TX2, TY2
};
static const uint32_t sPlaneIndices[] =
{ 0, 1, 2, 1, 3, 2 };
#else
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
#endif

CpuSerialParticleSystem::CpuSerialParticleSystem(uint32_t maxParticles)
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
	mParticleRenderData.resize(CpuRenderParticle::ParticleSize * mNumVertices * mNumMaxParticles);
#endif
}

CpuSerialParticleSystem::~CpuSerialParticleSystem()
{
	if (mVeo != 0)
	{
		glDeleteVertexArrays(1, &mVeo);
	}
	if (mVao != 0)
	{
		glDeleteVertexArrays(1, &mVao);
	}
	if (mVbo != 0)
	{
		glDeleteBuffers(1, &mVbo);
	}
	for (size_t i = 0; i < mModules.size(); i++)
	{
		delete mModules[i];
	}
	CHECK_GL_ERROR();
}

bool CpuSerialParticleSystem::Init()
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

void CpuSerialParticleSystem::BuildParticleVertexData()
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

void CpuSerialParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
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
		mParticles[i].Position += mParticles[i].Velocity * deltaTime;

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

void CpuSerialParticleSystem::RenderParticles()
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