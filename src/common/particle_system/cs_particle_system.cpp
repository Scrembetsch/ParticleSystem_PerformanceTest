#include "cs_particle_system.h"
#include "../gl/gl_util.h"

#include <sstream>

#define CREATE_BUFFER(val, size, type, attribPtr, attribSize) \
glGenBuffers(1, &val);  \
glBindBuffer(GL_SHADER_STORAGE_BUFFER, val); \
glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(type), NULL, GL_STATIC_DRAW); \
glEnableVertexAttribArray(attribPtr); \
glVertexAttribPointer(attribPtr, attribSize, GL_FLOAT, GL_FALSE, sizeof(type), (void*)0) \

#define INIT_BUFFER_BEGIN(size, type) \
{ \
    type* buffer = static_cast<type*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size * sizeof(type), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT)); \
    for (uint32_t i = 0; i < mNumMaxParticles; i++) \
    {
#define INIT_BUFFER_END() \
    } \
} \
glUnmapBuffer(GL_SHADER_STORAGE_BUFFER)

uint32_t atomicSize = sizeof(uint32_t) * 2;

CsParticleSystem::CsParticleSystem(uint32_t maxParticles, uint32_t groupSize)
    : mVao(0)
    , mAtomicBuffer(0)
    , mPosSsbo(0)
    , mVelSsbo(0)
    , mColSsbo(0)
    , mLifeSsbo(0)
    , mNumMaxParticles(maxParticles)
    , mNumParticles(0)
    , mLocalWorkGroupSize(groupSize, 1, 1)
    , mProjection(0)
    , mView(0)
    , mQuad1(0)
    , mQuad2(0)
    , mMinLifetime(0.0f)
    , mMaxLifetime(0.0f)
    , mMinStartVelocity(0.0f)
    , mMaxStartVelocity(0.0f)
    , mCurrentGenerateOffset(0.0f)
{
}

CsParticleSystem::~CsParticleSystem()
{
    if (mAtomicBuffer != 0)
    {
        glDeleteBuffers(1, &mAtomicBuffer);
        mAtomicBuffer = 0;
    }
    if (mPosSsbo != 0)
    {
        glDeleteBuffers(1, &mPosSsbo);
        mPosSsbo = 0;
    }
    if (mVelSsbo != 0)
    {
        glDeleteBuffers(1, &mVelSsbo);
        mVelSsbo = 0;
    }
    if (mColSsbo != 0)
    {
        glDeleteBuffers(1, &mColSsbo);
        mColSsbo = 0;
    }
    if (mLifeSsbo != 0)
    {
        glDeleteBuffers(1, &mLifeSsbo);
        mLifeSsbo = 0;
    }
    if (mVao != 0)
    {
        glDeleteVertexArrays(1, &mVao);
        mVao = 0;
    }
}

bool CsParticleSystem::Init()
{
    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mAtomicBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBuffer);
    glVertexAttribPointer(0, (atomicSize / sizeof(uint32_t)), GL_UNSIGNED_INT, GL_FALSE, atomicSize, 0);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, atomicSize, nullptr, GL_DYNAMIC_DRAW);
    ResetGenerateCounter();

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    CREATE_BUFFER(mPosSsbo, mNumMaxParticles, glm::vec3, 1, 3);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec3)
        buffer[i].x = 0.0f;
        buffer[i].y = 0.0f;
        buffer[i].z = 0.0f;
    INIT_BUFFER_END();

    CREATE_BUFFER(mVelSsbo, mNumMaxParticles, glm::vec3, 2, 3);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec3)
        buffer[i].x = sin(mRandom.Rand01() * 3.14f);
        buffer[i].y = cos(mRandom.Rand01() * 3.14f);
        buffer[i].z = sin(mRandom.Rand01() * 3.14f);
    INIT_BUFFER_END();

    CREATE_BUFFER(mColSsbo, mNumMaxParticles, glm::vec4, 3, 4);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec4)
        buffer[i].r = mRandom.Rand01();
        buffer[i].g = mRandom.Rand01();
        buffer[i].b = mRandom.Rand01();
        buffer[i].a = 1.0f;
    INIT_BUFFER_END();

    CREATE_BUFFER(mLifeSsbo, mNumMaxParticles, glm::vec2, 4, 2);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec2)
        buffer[i][0] = 0.0f;
        buffer[i][1] = 0.0f;
    INIT_BUFFER_END();

    std::stringstream ss;
    ss << mLocalWorkGroupSize.x;

    std::vector<std::pair<std::string, std::string>> replaceParts;
    replaceParts.emplace_back("LOCAL_SIZE_X", ss.str());

    bool success = true;
    success &= mComputeShader.LoadAndCompile("shader/cs_particle/basic.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mComputeShader.AttachLoadedShaders();
    success &= mComputeShader.Link();
    success &= mSortShader.LoadAndCompile("shader/cs_particle/sort.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mSortShader.AttachLoadedShaders();
    success &= mSortShader.Link();
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.vs", Shader::SHADER_TYPE_VERTEX);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.gs", Shader::SHADER_TYPE_GEOMETRY);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT, mRenderFsMap);
    success &= mRenderShader.AttachLoadedShaders();
    success &= mRenderShader.Link();

    return success;
}

void CsParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBuffer);
    ResetGenerateCounter();
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, mAtomicBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mPosSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mVelSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mColSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mLifeSsbo);

    float timeForParticle = 1.0f / EmitRate;

    mCurrentGenerateOffset += deltaTime;
    uint32_t numToGenerate = 0;

    // Todo: Could improve
    while (mCurrentGenerateOffset >= timeForParticle)
    {
        numToGenerate++;
        mCurrentGenerateOffset -= timeForParticle;
    }

    mComputeShader.Use();
    mComputeShader.SetFloat("uDeltaTime", deltaTime);
    mComputeShader.SetUInt("uNumToGenerate", numToGenerate);

    GLint bufMask = GL_MAP_READ_BIT;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLifeSsbo);
    auto* buffer = static_cast<glm::vec2*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, mNumMaxParticles * sizeof(glm::vec2), bufMask));
    for (int i = 0; i < mNumMaxParticles; i++)
    {
        auto newBuf = *buffer;
        //if (newBuf.x > 0.0f)
        {
            LOG("Before", " % d, % g, % g", i, newBuf.x, newBuf.y);
        }
        //LOG("CS", "%g, %g, %g", newBuf.x, newBuf.y, newBuf.z);
        //LOG("CS", "%g, %g, %g, %g", newBuf.x, newBuf.y, newBuf.z, newBuf.a);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glDispatchCompute(mNumMaxParticles / mLocalWorkGroupSize.x, 1, 1);

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    uint32_t* atomicPtr = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, atomicSize, GL_MAP_READ_BIT));
    mNumParticles = atomicPtr[1];
    LOG("Particles", "NUmGen: %d, Part: %d", atomicPtr[0], atomicPtr[1]);
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    //Sort();

    //GLint bufMask = GL_MAP_READ_BIT;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLifeSsbo);
    buffer = static_cast<glm::vec2*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, mNumMaxParticles * sizeof(glm::vec2), bufMask));
    for (int i = 0; i < mNumMaxParticles; i++)
    {
        auto newBuf = *buffer;
        //if (newBuf.x > 0.0f)
        {
            LOG("After", "%d, %g, %g", i, newBuf.x, newBuf.y);
        }
        //LOG("CS", "%g, %g, %g", newBuf.x, newBuf.y, newBuf.z);
        //LOG("CS", "%g, %g, %g, %g", newBuf.x, newBuf.y, newBuf.z, newBuf.a);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CsParticleSystem::Sort()
{
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

void CsParticleSystem::SortLocalBms(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 0);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(mNumMaxParticles / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::SortLocalDisperse(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 1);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(mNumMaxParticles / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::SortBigFlip(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 2);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(mNumMaxParticles / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::SortBigDisperse(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 3);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(mNumMaxParticles / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::PrepareRender(Camera* camera)
{
    mProjection = glm::perspective(glm::radians(camera->Zoom), camera->ViewWidth / camera->ViewHeight, 0.1f, 200.0f);
    mView = camera->GetViewMatrix();

    mQuad1 = glm::normalize(glm::cross(camera->Front, camera->Up));
    mQuad2 = glm::normalize(glm::cross(camera->Front, mQuad1));
}

void CsParticleSystem::RenderParticles()
{
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mRenderShader.Use();
    mRenderShader.SetMat4("uProjection", mProjection);
    mRenderShader.SetMat4("uView", mView);
    mRenderShader.SetVec3("uQuad1", mQuad1);
    mRenderShader.SetVec3("uQuad2", mQuad2);

    glBindVertexArray(mVao);
    glDrawArrays(GL_POINTS, 0, mNumMaxParticles);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

uint32_t CsParticleSystem::GetCurrentParticles() const
{
    return mNumParticles;
}

void CsParticleSystem::SetMinLifetime(float minLifetime)
{
    mMinLifetime = minLifetime;
}

void CsParticleSystem::SetMaxLifetime(float maxLifetime)
{
    mMaxLifetime = maxLifetime;
}

void CsParticleSystem::SetMinStartVelocity(const glm::vec3& minVelocity)
{
    mMinStartVelocity = minVelocity;
}

void CsParticleSystem::SetMaxStartVelocity(const glm::vec3& maxVelocity)
{
    mMaxStartVelocity = maxVelocity;
}

void CsParticleSystem::SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap)
{
    mRenderFsMap = replaceMap;
}

Shader* CsParticleSystem::GetRenderShader()
{
    return &mRenderShader;
}

void CsParticleSystem::ResetGenerateCounter()
{
    uint32_t* atomicPtr = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, atomicSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    atomicPtr[0] = 0;
    atomicPtr[1] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}