#include "tf_particle_system.h"

#include "../glm/gtc/matrix_transform.hpp"
#include "../gl/gl_util.h"
#include <random>

TfParticleSystem::TfParticleSystem()
    : mNextGenerationTime(0.0f)
    , mTransformFeedbackBuffer(0)
    , mQuery(0)
    , mCurrentReadBuffer(0)
    , mNumParticles(0)
    , mProjection(0)
    , mView(0)
    , mQuad1(0)
    , mQuad2(0)
    , mElapsedTime(0.0f)
    , mPosition(0)
    , mVelocityMin(0)
    , mVelocityRange(5)
    , mGravity(0)
    , mColor(1)
    , mLifeTimeMin(5)
    , mLifeTimeRange(2)
    , mSize(1)
    , mNumToGenerate(100)
{
    for (uint32_t i = 0; i < sBufferSize; i++)
    {
        mVbos[i] = 0;
        mVaos[i] = 0;
    }
}

TfParticleSystem::~TfParticleSystem()
{
    if (mTransformFeedbackBuffer != 0)
    {
        glDeleteTransformFeedbacks(1, &mTransformFeedbackBuffer);
    }
    if (mQuery != 0)
    {
        glDeleteQueries(1, &mQuery);
    }
    if (mVbos[0] != 0)
    {
        glDeleteBuffers(sBufferSize, mVbos);
    }
    if (mVaos[0] != 0)
    {
        glDeleteVertexArrays(sBufferSize, mVaos);
    }
}

bool TfParticleSystem::Init()
{
    const char* sVaryings[] = {
            "vPositionOut",
            "vVelocityOut",
            "vColorOut",
            "vLifeTimeOut",
            "vSizeOut",
            "vTypeOut"
    };
    unsigned int varyingSize = sizeof(sVaryings) / sizeof(sVaryings[0]);
    bool success = true;
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.vs", Shader::SHADER_TYPE_VERTEX);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.gs", Shader::SHADER_TYPE_GEOMETRY);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mUpdateShader.AttachLoadedShaders();

    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.vs", Shader::SHADER_TYPE_VERTEX);
    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.gs", Shader::SHADER_TYPE_GEOMETRY);
    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mRenderShader.AttachLoadedShaders();
    success &= mRenderShader.Link();
    if (!success)
    {
        return false;
    }

    glTransformFeedbackVaryings(mUpdateShader.GetId(), varyingSize, sVaryings, GL_INTERLEAVED_ATTRIBS);

    if (!mUpdateShader.Link())
    {
        return false;
    }

    glGenTransformFeedbacks(1, &mTransformFeedbackBuffer);
    glGenQueries(1, &mQuery);

    glGenBuffers(sBufferSize, mVbos);
    glGenVertexArrays(sBufferSize, mVaos);

    Particle initParticle;
    initParticle.Type = 0.0f;

    for (int i = 0; i < sBufferSize; i++)
    {
        glBindVertexArray(mVaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * sMaxParticles, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle);

        for (unsigned int j = 0; j < varyingSize; j++)
        {
            glEnableVertexAttribArray(j);
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)24);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)36);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)40);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)44);
    }
    glBindVertexArray(0);
    mCurrentReadBuffer = 0;
    mNumParticles = 1;

    return true;
}

void TfParticleSystem::UpdateParticles(float timeStep)
{
    mUpdateShader.Use();

    mUpdateShader.SetVec3("uPosition", mPosition);
    mUpdateShader.SetVec3("uVelocityMin", mVelocityMin);
    mUpdateShader.SetVec3("uVelocityRange", mVelocityRange);
    mUpdateShader.SetVec3("uColor", mColor);
    mUpdateShader.SetVec3("uGravity", mGravity);

    mUpdateShader.SetFloat("uTimeStep", timeStep);
    mUpdateShader.SetFloat("uLifeTimeMin", mLifeTimeMin);
    mUpdateShader.SetFloat("uLifeTimeRange", mLifeTimeRange);
    mUpdateShader.SetFloat("uSize", mSize);

    mElapsedTime += timeStep;

    if (mElapsedTime > mNextGenerationTime)
    {
        mUpdateShader.SetInt("uNumToGenerate", mNumToGenerate);
        mElapsedTime -= mNextGenerationTime;
        glm::vec3 randomSeed = glm::vec3(mRng.Rand(-10.0f, 20.0f), mRng.Rand(-10.0f, 20.0f), mRng.Rand(-10.0f, 20.0f));
        mUpdateShader.SetVec3("uRandomSeed", randomSeed);
    }
    else
    {
        mUpdateShader.SetInt("uNumToGenerate", 0);
    }
    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbackBuffer);

    glBindVertexArray(mVaos[mCurrentReadBuffer]);
    glEnableVertexAttribArray(1);

    // 1 -  use other buffer
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVbos[1 - mCurrentReadBuffer]);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mQuery);
    glBeginTransformFeedback(GL_POINTS);

    glDrawArrays(GL_POINTS, 0, mNumParticles);

    glEndTransformFeedback();
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glGetQueryObjectuiv(mQuery, GL_QUERY_RESULT, &mNumParticles);

    // toggle buffer
    mCurrentReadBuffer = 1 - mCurrentReadBuffer;

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glDisable(GL_RASTERIZER_DISCARD);
}

void TfParticleSystem::RenderParticles()
{
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    mRenderShader.Use();
    mRenderShader.SetMat4("uProjection", mProjection);
    mRenderShader.SetMat4("uView", mView);
    mRenderShader.SetVec3("uQuad1", mQuad1);
    mRenderShader.SetVec3("uQuad2", mQuad2);

    glBindVertexArray(mVaos[mCurrentReadBuffer]);
    glDisableVertexAttribArray(1);

    glDrawArrays(GL_POINTS, 0, mNumParticles);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void TfParticleSystem::SetGeneratorProperties(const glm::vec3& position, const glm::vec3& velocityMin, const glm::vec3& velocityMax, const glm::vec3& gravity, const glm::vec3 color, float minLifeTime, float maxLifeTime, float size, float spawnTime, int numToGenerate)
{
    mPosition = position;
    mVelocityMin = velocityMin;
    mVelocityRange = velocityMax - velocityMin;
    mGravity = gravity;
    mColor = color;

    mSize = size;
    mLifeTimeMin = minLifeTime;
    mLifeTimeRange = maxLifeTime - minLifeTime;

    mNextGenerationTime = spawnTime;
    mElapsedTime = 0.0f;

    mNumToGenerate = numToGenerate;
}

void TfParticleSystem::SetGeneratorPosition(const glm::vec3& position)
{
    mPosition = position;
}

uint32_t TfParticleSystem::GetNumParticles() const
{
    return mNumParticles;
}

void TfParticleSystem::SetMatrices(const glm::mat4& projection, const glm::mat4& viewMat, const glm::vec3& view, const glm::vec3& upVector)
{
    mProjection = projection;
    mView = viewMat;
    mQuad1 = glm::cross(view, upVector);
    mQuad1 = glm::normalize(mQuad1);
    mQuad2 = glm::cross(view, mQuad1);
    mQuad2 = glm::normalize(mQuad2);
}
