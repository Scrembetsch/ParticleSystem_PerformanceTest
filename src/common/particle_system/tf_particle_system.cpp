#include "tf_particle_system.h"

#include "../glm/gtc/matrix_transform.hpp"
#include "../gl/gl_util.h"
#include <random>

static const float sBasePlaneVertexData[] =
{
    // Coord
    -1.0f,   -1.0f,
    1.0f,   -1.0f,
    -1.0f,   1.0f,
    1.0f,   1.0f,
};

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
{
#if SORT
    mResolutionX = sqrt(maxParticles);
    mResolutionY = mResolutionX;
#endif

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
    if (mUpdateVbo != 0)
    {
        glDeleteBuffers(1, &mUpdateVbo);
        mUpdateVbo = 0;
    }
    if (mUpdateVao != 0)
    {
        glDeleteVertexArrays(1, &mUpdateVao);
        mUpdateVao = 0;
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
    replaceParts.emplace_back("MAX_PARTICLES", std::to_string(mNumMaxParticles));
    replaceParts.emplace_back("DECL_TEX1", "layout (binding = 1) uniform sampler2D uIndexMap;");
    replaceParts.emplace_back("USE_TEX1", "uIndexMap");

    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.vs", Shader::SHADER_TYPE_VERTEX, replaceMapVs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.gs", Shader::SHADER_TYPE_GEOMETRY, replaceMapGs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mUpdateShader.AttachLoadedShaders();

    //success &= mSortShader.LoadAndCompile("shader/tf_particle/sort.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    //success &= mSortShader.AttachLoadedShaders();
    //success &= mSortShader.Link();

#if SORT
    glGenFramebuffers(1, &mSortBuffer);
    for (uint32_t i = 0; i < 2; i++)
    {
        glGenTextures(1, &mIndexTex[i].mTex);
        mIndexTex[i].mTexName = "uIndexMap";
        mIndexTex[i].mTexLocation = GL_TEXTURE1;
        glBindTexture(GL_TEXTURE_2D, mIndexTex[i].mTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, mResolutionX, mResolutionY, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    }

    success &= mPrepSortShader.LoadAndCompile("shader/tf_particle/prep_sort.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
    success &= mPrepSortShader.LoadAndCompile("shader/tf_particle/prep_sort.fs", Shader::SHADER_TYPE_FRAGMENT, replaceParts);
    success &= mPrepSortShader.AttachLoadedShaders();
    success &= mPrepSortShader.Link();

    success &= mSortShader.LoadAndCompile("shader/tf_particle/sort.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
    success &= mSortShader.LoadAndCompile("shader/tf_particle/sort.fs", Shader::SHADER_TYPE_FRAGMENT, replaceParts);
    success &= mSortShader.AttachLoadedShaders();
    success &= mSortShader.Link();

    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render_sort.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
#else
    success &= mRenderShader.LoadAndCompile("shader/tf_particle/render.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
#endif
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

    glGenVertexArrays(1, &mUpdateVao);
    glBindVertexArray(mUpdateVao);
    CHECK_GL_ERROR();

    glGenBuffers(1, &mUpdateVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mUpdateVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sBasePlaneVertexData), sBasePlaneVertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);

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

    // toggle buffer
    mCurrentReadBuffer = 1 - mCurrentReadBuffer;
    mCurrentWriteBuffer = 1 - mCurrentReadBuffer;
    mSortCurrentReadBuffer = mCurrentReadBuffer;
    mSortCurrentWriteBuffer = mCurrentWriteBuffer;

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

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVbos[mCurrentWriteBuffer]);

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

#if SORT
    Sort();

    CHECK_GL_ERROR();
#endif
}

void TfParticleSystem::Sort()
{
    OPTICK_EVENT();
    CHECK_GL_ERROR();

    // Save State
    int32_t val = 0;
#if _WIN32
    glGetIntegerv(GL_DRAW_BUFFER, &val);
#else
    glGetIntegerv(GL_DRAW_BUFFER0, &val);
#endif
    int32_t viewPortDims[4];
    glGetIntegerv(GL_VIEWPORT, viewPortDims);
    glViewport(0, 0, mResolutionX, mResolutionY);
    CHECK_GL_ERROR();

    PrepSort();

    mSortShader.Use();
    CHECK_GL_ERROR();
    mSortShader.SetVec2("uResolution", mResolutionX, mResolutionY);
    CHECK_GL_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mSortBuffer);
    CHECK_GL_ERROR();
    unsigned int baseattachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, baseattachments);
    CHECK_GL_ERROR();

    for (uint32_t stageDistance = 1; stageDistance < mNumMaxParticles; stageDistance *= 2)
    {
        for (uint32_t stepDistance = stageDistance; stepDistance > 0; stepDistance /= 2)
        {
            mSortCurrentReadBuffer = 1 - mSortCurrentReadBuffer;
            mSortCurrentWriteBuffer = 1 - mSortCurrentReadBuffer;

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mIndexTex[mSortCurrentWriteBuffer].mTex, 0);
            CHECK_GL_ERROR();

            mIndexTex[mSortCurrentReadBuffer].Use(&mSortShader);
            mSortShader.SetUInt("uStageDistance", stageDistance);
            mSortShader.SetUInt("uStepDistance", stepDistance);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            CHECK_GL_ERROR();
        }
    }


    // Restore State
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    unsigned int resetattach[1] = { GL_NONE };
    resetattach[0] = val;
    glDrawBuffers(1, resetattach);
    glViewport(viewPortDims[0], viewPortDims[1], viewPortDims[2], viewPortDims[3]);
}

void TfParticleSystem::PrepSort()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mSortBuffer);
    unsigned int baseattachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, baseattachments);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, mVbos[mCurrentWriteBuffer]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mIndexTex[mSortCurrentWriteBuffer].mTex, 0);

    mPrepSortShader.Use();
    mPrepSortShader.SetVec2("uResolution", mResolutionX, mResolutionY);
    mPrepSortShader.SetUInt("uNumAliveParticles", mNumParticles);

    glBindVertexArray(mUpdateVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, mVbos[mCurrentWriteBuffer]);

#if SORT
    mRenderShader.SetVec2("uResolution", mResolutionX, mResolutionY);
    mIndexTex[mSortCurrentWriteBuffer].Use(&mRenderShader);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mNumParticles - GetEmitters());
#else
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mNumParticles);
#endif
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