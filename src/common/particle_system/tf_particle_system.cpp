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
    , mMaxVertices(0)
    , mNumEmitters(0)
    , mProjection(0)
    , mView(0)
    , mQuad1(0)
    , mQuad2(0)
    , mMinLifetime(0.0f)
    , mMaxLifetime(0.0f)
    , mMinStartVelocity(0.0f)
    , mMaxStartVelocity(0.0f)
    , mLocalWorkGroupSize(256, 0, 0)
    , mSortSsbo(0)
{
    for (uint32_t i = 0; i < sBufferSize; i++)
    {
        mVbos[i] = 0;
        mVaos[i] = 0;
    }

    int32_t maxVertices = 0;
    int32_t maxComponents = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxVertices);
    glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &maxComponents);

    int32_t maxVerticesByComponentLimit = maxComponents / (sizeof(TfParticle) / sizeof(float));
    mMaxVertices = std::min(maxVertices, maxVerticesByComponentLimit);
    // Somehow AMD cannot handle reported maxVertices
    mMaxVertices = std::min(mMaxVertices, 128);
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

    if (mSortSsbo != 0)
    {
        glDeleteBuffers(1, &mSortSsbo);
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
            "vDataOut"
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
    replaceMapGs.emplace_back("MAX_OUTPUT_VERTICES", std::to_string(mMaxVertices));

    replaceMapVs.emplace_back("MODULE_CALLS", callsVs);
    replaceMapGs.emplace_back("MODULE_CALLS", callsGs);

    std::vector<std::pair<std::string, std::string>> replaceParts;
    replaceParts.emplace_back("LOCAL_SIZE_X", std::to_string(mLocalWorkGroupSize.x));

    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.vs", Shader::SHADER_TYPE_VERTEX, replaceMapVs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.gs", Shader::SHADER_TYPE_GEOMETRY, replaceMapGs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mUpdateShader.AttachLoadedShaders();

    success &= mSortShader.LoadAndCompile("shader/tf_particle/sort.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mSortShader.AttachLoadedShaders();
    success &= mSortShader.Link();

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

    mNumEmitters = static_cast<uint32_t>(std::ceil(static_cast<float>(mNumMaxParticles) / static_cast<float>(mMaxVertices)));

    TfParticle* initParticles = new TfParticle[mNumEmitters];
    for (uint32_t i = 0; i < mNumEmitters; i++)
    {
        initParticles[i].Data.z = (i + 1);
    }

    for (uint32_t i = 0; i < sBufferSize; i++)
    {
        glBindVertexArray(mVaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TfParticle) * mNumMaxParticles, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TfParticle) * mNumEmitters, initParticles);

        for (uint32_t j = 0; j < varyingSize; j++)
        {
            glEnableVertexAttribArray(j);
        }

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)16);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)32);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(TfParticle), (void*)48);
    }

    glBindVertexArray(0);
    mCurrentReadBuffer = 0;
    mNumParticles = mNumEmitters;
    delete[] initParticles;

#if SORT
    glGenBuffers(1, &mSortSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSortSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TfParticle) * mNumMaxParticles, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(TfParticle), nullptr);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif

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
    OPTICK_EVENT();

    mUpdateShader.Use();

    mUpdateShader.SetVec3("uPosition", glm::vec3(0.0f));
    mUpdateShader.SetVec3("uCameraPos", cameraPos);
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

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);
    CHECK_GL_ERROR();

    // toggle buffer
    mCurrentReadBuffer = 1 - mCurrentReadBuffer;

#if SORT
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mVbos[mCurrentReadBuffer]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mVbos[mCurrentReadBuffer]);

    Sort();

    CHECK_GL_ERROR();
#endif

}

void TfParticleSystem::Sort()
{
    OPTICK_EVENT();

    mSortShader.Use();
    uint32_t h = mLocalWorkGroupSize.x * 2;

    SortLocalBms(mNumMaxParticles, h);

    // we must now double h, as this happens before every flip
    h *= 2;

    for (; h <= mNumMaxParticles; h *= 2) {

        SortBigFlip(mNumMaxParticles, h);

        for (uint32_t hh = h / 2; hh > 1; hh /= 2) {

            if (hh <= mLocalWorkGroupSize.x * 2) {
                SortLocalDisperse(mNumMaxParticles, hh);
                break;
            }
            else {
                SortBigDisperse(mNumMaxParticles, hh);
            }
        }
    }
}

void TfParticleSystem::SortLocalBms(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 0);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
}

void TfParticleSystem::SortLocalDisperse(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 1);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
}

void TfParticleSystem::SortBigFlip(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 2);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
}

void TfParticleSystem::SortBigDisperse(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 3);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
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
    OPTICK_EVENT();

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

int32_t TfParticleSystem::GetMaxVerticesPerEmitter() const
{
    return mMaxVertices;
}

uint32_t TfParticleSystem::GetEmitters() const
{
    return mNumEmitters;
}