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
static const CpuRenderParticle sBasePlaneVertexData[] =
{
    // Coord			// Color				// Tex Coord
    CpuRenderParticle{glm::vec3(-0.5, -0.5,  0.0),	glm::Vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX1, TY1)},
    CpuRenderParticle{glm::vec3(0.5, -0.5,  0.0),	glm::Vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX2, TY1)},
    CpuRenderParticle{glm::vec3(-0.5,  0.5,  0.0),	glm::Vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX1, TY2)},
    CpuRenderParticle{glm::vec3(0.5,  0.5,  0.0),	glm::Vec4(1.0, 1.0, 1.0, 1.0),		glm::vec2(TX2, TY2)}
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
    mParticleRenderData.resize(mNumVertices * mNumMaxParticles);
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
            mModules[j]->UpdateParticle(deltaTime, particle);
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