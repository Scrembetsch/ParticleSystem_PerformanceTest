#include "fs_particle_system.h"
#include "../gl/gl_util.h"

#include <sstream>

static const float sBasePlaneVertexData[] =
{
    // Coord
    -1.0f,   -1.0f,
    1.0f,   -1.0f,
    -1.0f,   1.0f,
    1.0f,   1.0f,
};

FsParticleSystem::FsParticleSystem(uint32_t maxParticles)
    : mEmptyVao(0)
    , mEmptyVbo(0)
    , mUpdateVao(0)
    , mUpdateVbo(0)
    , mResolutionX(0)
    , mResolutionY(0)
    , mNumMaxParticles(maxParticles)
    , mNumParticles(0)
    , mProjection(0)
    , mView(0)
    , mQuad1(0)
    , mQuad2(0)
    , mMinLifetime(0.0f)
    , mMaxLifetime(0.0f)
    , mCurrentReadBuffer(0)
    , mCurrentWriteBuffer(1)
    , mMinStartVelocity(0.0f)
    , mMaxStartVelocity(0.0f)
    , mCurrentTime(0.0f)
{
    uint32_t sqrt = std::sqrt(maxParticles);
    if (sqrt * sqrt == maxParticles)
    {
        mResolutionX = sqrt;
        mResolutionY = sqrt;
    }
}

FsParticleSystem::~FsParticleSystem()
{
    if (mEmptyVbo != 0)
    {
        glDeleteBuffers(1, &mEmptyVbo);
        mEmptyVbo = 0;
    }
    if (mEmptyVao != 0)
    {
        glDeleteVertexArrays(1, &mEmptyVao);
        mEmptyVao = 0;
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
    for (uint32_t i = 0; i < 2; i++)
    {
        glDeleteFramebuffers(1, &mFramebuffer[i]);
        mFramebuffer[i] = 0;
    }
}

bool FsParticleSystem::Init()
{
    glGenVertexArrays(1, &mEmptyVao);
    glBindVertexArray(mEmptyVao);

    glGenBuffers(1, &mEmptyVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mEmptyVbo);
    float* tempData = new float[mNumMaxParticles * 6];
    glBufferData(GL_ARRAY_BUFFER, sizeof(tempData), tempData, GL_STATIC_DRAW);
    delete tempData;

    glGenVertexArrays(1, &mUpdateVao);
    glBindVertexArray(mUpdateVao);
    CHECK_GL_ERROR();

    glGenBuffers(1, &mUpdateVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mUpdateVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sBasePlaneVertexData), sBasePlaneVertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);

    for (uint32_t i = 0; i < 2; i++)
    {
        glGenFramebuffers(1, &mFramebuffer[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer[i]);
        CHECK_GL_ERROR();

        glGenTextures(1, &mPosition[i].mTex);
        mPosition[i].mTexName = "uPositionMap";
        mPosition[i].mTexLocation = GL_TEXTURE1;
        glBindTexture(GL_TEXTURE_2D, mPosition[i].mTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResolutionX, mResolutionY, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPosition[i].mTex, 0);
        CHECK_GL_ERROR();

        glGenTextures(1, &mVelocity[i].mTex);
        mVelocity[i].mTexName = "uVelocityMap";
        mVelocity[i].mTexLocation = GL_TEXTURE2;
        glBindTexture(GL_TEXTURE_2D, mVelocity[i].mTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResolutionX, mResolutionY, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mVelocity[i].mTex, 0);
        CHECK_GL_ERROR();

        glGenTextures(1, &mColor[i].mTex);
        mColor[i].mTexName = "uColorMap";
        mColor[i].mTexLocation = GL_TEXTURE3;
        glBindTexture(GL_TEXTURE_2D, mColor[i].mTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mResolutionX, mResolutionY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mColor[i].mTex, 0);
        CHECK_GL_ERROR();


    }
    CHECK_GL_ERROR();

    mData0Array = new glm::vec4[mNumMaxParticles];
    for (uint32_t i = 0; i < mNumMaxParticles; i++)
    {
        mData0Array[i] = glm::vec4(0.0f);
        mGenerateQueue.push(i);
    }

    glGenTextures(1, &mData0.mTex);
    CHECK_GL_ERROR();
    mData0.mTexName = "uData0Map";
    mData0.mTexLocation = GL_TEXTURE4;

    glBindTexture(GL_TEXTURE_2D, mData0.mTex);
    CHECK_GL_ERROR();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResolutionX, mResolutionY, 0, GL_RGBA, GL_FLOAT, mData0Array);
    CHECK_GL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    CHECK_GL_ERROR();

    std::vector<std::pair<std::string, std::string>> replaceParts;
    for (auto it = mRenderFsMap.begin(); it != mRenderFsMap.end(); it++)
    {
        replaceParts.emplace_back(it->first, it->second);
    }
    replaceParts.emplace_back("DECL_TEX1", "layout (binding = 1) uniform sampler2D uPositionMap;");
    replaceParts.emplace_back("USE_TEX1", "uPositionMap");
    replaceParts.emplace_back("DECL_TEX2", "layout (binding = 2) uniform sampler2D uVelocityMap;");
    replaceParts.emplace_back("USE_TEX2", "uVelocityMap");
    replaceParts.emplace_back("DECL_TEX3", "layout (binding = 3) uniform sampler2D uColorMap;");
    replaceParts.emplace_back("USE_TEX3", "uColorMap");
    replaceParts.emplace_back("DECL_TEX4", "layout (binding = 4) uniform sampler2D uData0Map;");
    replaceParts.emplace_back("USE_TEX4", "uData0Map");

    std::string moduleMethods;
    std::string moduleCalls;
    std::string moduleUniforms;

    for (size_t i = 0; i < mModules.size(); i++)
    {
        moduleMethods += mModules[i]->GetModuleMethods();
        moduleCalls += mModules[i]->GetMethodCall();
        moduleUniforms += mModules[i]->GetUniforms();
    }

    replaceParts.emplace_back("MODULE_UNIFORMS", moduleUniforms);
    replaceParts.emplace_back("MODULE_METHODS", moduleMethods);
    replaceParts.emplace_back("MODULE_CALLS", moduleCalls);

    bool success = true;
    success &= mUpdateShader.LoadAndCompile("shader/fs_particle/update.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
    success &= mUpdateShader.LoadAndCompile("shader/fs_particle/update.fs", Shader::SHADER_TYPE_FRAGMENT, replaceParts);
    success &= mUpdateShader.AttachLoadedShaders();
    success &= mUpdateShader.Link();
    success &= mRenderShader.LoadAndCompile("shader/fs_particle/render.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
    success &= mRenderShader.LoadAndCompile("shader/fs_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT, replaceParts);
    success &= mRenderShader.AttachLoadedShaders();
    success &= mRenderShader.Link();
    CHECK_GL_ERROR();


    return success;
}

void FsParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
    mCurrentTime += deltaTime;

    int32_t val = 0;
    glGetIntegerv(GL_DRAW_BUFFER, &val);

    int32_t viewPortDims[4];
    glGetIntegerv(GL_VIEWPORT, viewPortDims);
    glViewport(0, 0, mResolutionX, mResolutionY);

    mCurrentReadBuffer = 1 - mCurrentReadBuffer;
    mCurrentWriteBuffer = 1 - mCurrentReadBuffer;

    CheckForDeadParticles();

    for (uint32_t i = 0; i < mModules.size(); i++)
    {
        mModules[i]->PreRun(mCurrentTime, deltaTime);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer[mCurrentWriteBuffer]);
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    CHECK_GL_ERROR();

    mUpdateShader.Use();
    mUpdateShader.SetFloat("uCurrentTime", mCurrentTime);
    mUpdateShader.SetFloat("uDeltaTime", deltaTime);
    mUpdateShader.SetVec3("uCameraPos", cameraPos);
    mUpdateShader.SetVec3("uPosition", glm::vec3(0.0f));
    mUpdateShader.SetVec3("uCameraPos", cameraPos);
    mUpdateShader.SetVec3("uRandomSeed", glm::vec3(mRandom.Rand(-20.0f, 20.0f), mRandom.Rand(-20.0f, 20.0f), mRandom.Rand(-20.0f, 20.0f)));
    mUpdateShader.SetVec3("uVelocityMin", mMinStartVelocity);
    mUpdateShader.SetVec3("uVelocityRange", mMaxStartVelocity - mMinStartVelocity);

    mUpdateShader.SetFloat("uLifeTimeMin", mMinLifetime);
    mUpdateShader.SetFloat("uLifeTimeRange", mMaxLifetime - mMinLifetime);

    for (uint32_t i = 0; i < mModules.size(); i++)
    {
        mModules[i]->ApplyShaderValues(mCurrentTime, deltaTime, &mUpdateShader);
    }

    mPosition[mCurrentReadBuffer].Use(&mUpdateShader);
    mVelocity[mCurrentReadBuffer].Use(&mUpdateShader);
    mColor[mCurrentReadBuffer].Use(&mUpdateShader);
    mData0.Use(&mUpdateShader);

    CHECK_GL_ERROR();

    glBindVertexArray(mUpdateVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR();

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int baseattachments[3] = { GL_NONE, GL_NONE, GL_NONE };
    glDrawBuffers(3, baseattachments);
    CHECK_GL_ERROR();

    glDrawBuffer(val);
    CHECK_GL_ERROR();

    glViewport(viewPortDims[0], viewPortDims[1], viewPortDims[2], viewPortDims[3]);

    CHECK_GL_ERROR();
}

void FsParticleSystem::PrepareRender(Camera* camera)
{
    mProjection = glm::perspective(glm::radians(camera->Zoom), camera->ViewWidth / camera->ViewHeight, 0.1f, 200.0f);
    mView = camera->GetViewMatrix();

    mQuad1 = glm::normalize(glm::cross(camera->Front, camera->Up));
    mQuad2 = glm::normalize(glm::cross(camera->Front, mQuad1));
}

void FsParticleSystem::RenderParticles()
{
    OPTICK_EVENT();

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mRenderShader.Use();
    mRenderShader.SetFloat("uCurrentTime", mCurrentTime);
    mRenderShader.SetVec2("uResolution", mResolutionX, mResolutionY);
    mRenderShader.SetMat4("uProjection", mProjection);
    mRenderShader.SetMat4("uView", mView);
    mRenderShader.SetVec3("uQuad1", mQuad1);
    mRenderShader.SetVec3("uQuad2", mQuad2);

    mPosition[mCurrentWriteBuffer].Use(&mRenderShader);
    mVelocity[mCurrentWriteBuffer].Use(&mRenderShader);
    mColor[mCurrentWriteBuffer].Use(&mRenderShader);
    mData0.Use(&mRenderShader);
    CHECK_GL_ERROR();

    glBindVertexArray(mEmptyVao);
    glDrawArrays(GL_TRIANGLES, 0, mNumMaxParticles * 6);
    CHECK_GL_ERROR();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    CHECK_GL_ERROR();
}

void FsParticleSystem::AddModule(FsIModule* mod)
{
    mModules.push_back(mod);
}

void FsParticleSystem::Emit(uint32_t numToGenerate)
{
    numToGenerate = std::min(numToGenerate, uint32_t(mGenerateQueue.size()));
    uint32_t generated = 0;

    uint32_t startId = 0;
    uint32_t startIdRow = 0;
    uint32_t lastId = 0;
    uint32_t length = 0;

    while (numToGenerate > 0 && !mGenerateQueue.empty())
    {
        bool update = false;
        startId = mGenerateQueue.front();
        mGenerateQueue.pop();
        startIdRow = startId / mResolutionX;
        lastId = startId;
        length = 1;
        numToGenerate--;

        while (!update && numToGenerate > 0 && !mGenerateQueue.empty())
        {
            uint32_t id = mGenerateQueue.front();
            if (id != lastId + 1
                || id / mResolutionX != startIdRow)
            {
                update = true;
                continue;
            }
            mGenerateQueue.pop();

            lastId = id;
            length++;
            numToGenerate--;

            if (numToGenerate == 0 || mGenerateQueue.empty())
                update = true;
        }

        generated += length;
        InitParticles(startId, startId + length);
    }

    //for (uint32_t i = 0; i < maxToGenerate; i++)
    //{
    //    uint32_t id = mGenerateQueue.front();
    //    mGenerateQueue.pop();
    //    int32_t x = id % mResolutionX;
    //    int32_t y = id / mResolutionY;

    //    mData0Array[id].x = mCurrentTime + mRandom.Rand(mMinLifetime, mMaxLifetime);
    //    mData0Array[id].y = mCurrentTime;

    //    glTextureSubImage2D(mData0.mTex, 0, x, y, 1, 1, GL_RGBA, GL_FLOAT, &mData0Array[id]);
    //    CHECK_GL_ERROR();

    //    generated++;
    //}
    mNumParticles += generated;
}

void FsParticleSystem::InitParticles(uint32_t fromId, uint32_t toId)
{
    uint32_t row = fromId / mResolutionX;
    uint32_t column = fromId % mResolutionX;

    for (uint32_t i = fromId; i < toId; i++)
    {
        mData0Array[i].x = mCurrentTime + mRandom.Rand(mMinLifetime, mMaxLifetime);
        mData0Array[i].y = mCurrentTime;
    }

    glTextureSubImage2D(mData0.mTex, 0, column, row, toId - fromId, 1, GL_RGBA, GL_FLOAT, &mData0Array[fromId]);
}

void FsParticleSystem::CheckForDeadParticles()
{
    uint32_t updatedParticles = 0;
    uint32_t removedParticles = 0;
    for (uint32_t i = 0; i < mNumMaxParticles && updatedParticles < mNumParticles; i++)
    {
        if (mData0Array[i].x == 0.0f)
            continue;

        if (mCurrentTime >= mData0Array[i].x)
        {
            mData0Array[i].x = 0.0f;
            mGenerateQueue.push(i);
            removedParticles--;
        }
        updatedParticles++;
    }
    mNumParticles -= removedParticles;
}

uint32_t FsParticleSystem::GetCurrentParticles() const
{
    return mNumParticles;
}

void FsParticleSystem::SetMinLifetime(float minLifetime)
{
    mMinLifetime = minLifetime;
}

void FsParticleSystem::SetMaxLifetime(float maxLifetime)
{
    mMaxLifetime = maxLifetime;
}

void FsParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
    mMinStartVelocity = minVelocity;
}

void FsParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
    mMaxStartVelocity = maxVelocity;
}

void FsParticleSystem::SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap)
{
    mRenderFsMap = replaceMap;
}

Shader* FsParticleSystem::GetRenderShader()
{
    return &mRenderShader;
}
