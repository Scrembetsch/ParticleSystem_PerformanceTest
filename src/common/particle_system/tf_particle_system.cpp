#include "tf_particle_system.h"

#include "../glm/gtc/matrix_transform.hpp"
#include "../gl/gl_util.h"
#include <random>

TfParticleSystem::TfParticleSystem(uint32_t maxParticles)
    : mTransformFeedbackBuffer(0)
    , mQuery(0)
    , mCurrentReadBuffer(0)
    , mNumMaxParticles(maxParticles)
    , mNumParticles(0)
    , mProjection(0)
    , mView(0)
    , mQuad1(0)
    , mQuad2(0)
    , mMinLifetime(0.0f)
    , mMaxLifetime(0.0f)
    , mMinStartVelocity(0.0f)
    , mMaxStartVelocity(0.0f)
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

    for (uint32_t i = 0; i < mModules.size(); i++)
    {
        delete mModules[i];
    }
    CHECK_GL_ERROR();
}

bool TfParticleSystem::Init()
{
    const char* sVaryings[] = {
            "vPositionOut",
            "vVelocityOut",
            "vColorOut",
            "vLifeTimeOut",
            "vLifeTimeBeginOut",
            "vTypeOut"
    };
    unsigned int varyingSize = sizeof(sVaryings) / sizeof(sVaryings[0]);
    bool success = true;

    std::vector<std::pair<std::string, std::string>> replaceMapVs;
    std::vector<std::pair<std::string, std::string>> replaceMapGs;

    std::string uniforms;

    std::string methodsVs;
    std::string methodsGs;

    std::string callsVs;
    std::string callsGs;
    for (uint32_t i = 0; i < mModules.size(); i++)
    {
        uniforms += mModules[i]->GetUniforms();

        methodsVs += mModules[i]->GetModuleMethods(Shader::ShaderType::SHADER_TYPE_VERTEX);
        methodsGs += mModules[i]->GetModuleMethods(Shader::ShaderType::SHADER_TYPE_GEOMETRY);

        callsVs += mModules[i]->GetMethodCall(Shader::ShaderType::SHADER_TYPE_VERTEX);
        callsGs += mModules[i]->GetMethodCall(Shader::ShaderType::SHADER_TYPE_GEOMETRY);
    }

    replaceMapVs.emplace_back("MODULE_UNIFORMS", uniforms);
    replaceMapGs.emplace_back("MODULE_UNIFORMS", uniforms);

    replaceMapVs.emplace_back("MODULE_METHODS", methodsVs);
    replaceMapGs.emplace_back("MODULE_METHODS", methodsGs);

    replaceMapVs.emplace_back("MODULE_CALLS", callsVs);
    replaceMapGs.emplace_back("MODULE_CALLS", callsGs);

    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.vs", Shader::SHADER_TYPE_VERTEX, replaceMapVs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.gs", Shader::SHADER_TYPE_GEOMETRY, replaceMapGs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mUpdateShader.AttachLoadedShaders();

    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.vs", Shader::SHADER_TYPE_VERTEX);
    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.gs", Shader::SHADER_TYPE_GEOMETRY);
    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT, mRenderFsMap);
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

    TfParticle initParticle;
    initParticle.Type = 0.0f;

    for (int i = 0; i < sBufferSize; i++)
    {
        glBindVertexArray(mVaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TfParticle) * mNumMaxParticles, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TfParticle), &initParticle);

        for (unsigned int j = 0; j < varyingSize; j++)
        {
            glEnableVertexAttribArray(j);
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)12);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)24);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)40);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)44);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)48);
    }
    glBindVertexArray(0);
    mCurrentReadBuffer = 0;
    mNumParticles = 1;

    CHECK_GL_ERROR();
    return true;
}

bool TfParticleSystem::AddModule(TfIModule* psModule)
{
    mModules.push_back(psModule);
    return true;
}

void TfParticleSystem::UpdateParticles(float timeStep, const glm::vec3& cameraPos)
{
    mUpdateShader.Use();

    mUpdateShader.SetVec3("uPosition", glm::vec3(0.0f));
    mUpdateShader.SetVec3("uVelocityMin", mMinStartVelocity);
    mUpdateShader.SetVec3("uVelocityRange", mMaxStartVelocity - mMinStartVelocity);

    mUpdateShader.SetFloat("uTimeStep", timeStep);
    mUpdateShader.SetFloat("uLifeTimeMin", mMinLifetime);
    mUpdateShader.SetFloat("uLifeTimeRange", mMaxLifetime - mMinLifetime);

    glm::vec3 randomSeed = glm::vec3(mRandom.Rand(-10.0f, 20.0f), mRandom.Rand(-10.0f, 20.0f), mRandom.Rand(-10.0f, 20.0f));
    mUpdateShader.SetVec3("uRandomSeed", randomSeed);

    for (uint32_t i = 0; i < mModules.size(); i++)
    {
        mModules[i]->ApplyUniforms(timeStep, &mUpdateShader);
    }

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbackBuffer);

    glBindVertexArray(mVaos[mCurrentReadBuffer]);

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

    CHECK_GL_ERROR();
}

void TfParticleSystem::PrepareRender(Camera* camera)
{
    mProjection = glm::perspective(glm::radians(camera->Zoom), camera->ViewWidth / camera->ViewHeight, 0.1f, 200.0f);
    mView = camera->GetViewMatrix();

    mQuad1 = glm::normalize(glm::cross(camera->Front, camera->Up));
    mQuad2 = glm::normalize(glm::cross(camera->Front, mQuad1));
}

void TfParticleSystem::RenderParticles()
{
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mRenderShader.Use();
    mRenderShader.SetMat4("uProjection", mProjection);
    mRenderShader.SetMat4("uView", mView);
    mRenderShader.SetVec3("uQuad1", mQuad1);
    mRenderShader.SetVec3("uQuad2", mQuad2);
    glBindVertexArray(mVaos[mCurrentReadBuffer]);

    glDrawArrays(GL_POINTS, 0, mNumParticles);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    CHECK_GL_ERROR();
}

uint32_t TfParticleSystem::GetCurrentParticles() const
{
    return mNumParticles;
}

void TfParticleSystem::SetMinLifetime(float minLifetime)
{
    mMinLifetime = minLifetime;
}

void TfParticleSystem::SetMaxLifetime(float maxLifetime)
{
    mMaxLifetime = maxLifetime;
}

void TfParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
    mMinStartVelocity = minVelocity;
}

void TfParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
    mMaxStartVelocity = maxVelocity;
}

void TfParticleSystem::SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap)
{
    mRenderFsMap = replaceMap;
}

Shader* TfParticleSystem::GetRenderShader()
{
    return &mRenderShader;
}