#include "cs_particle_system.h"
#include "../gl/gl_util.h"

#include <sstream>

#define CREATE_BUFFER(val, size, attribPtr, type) \
glGenBuffers(1, &val);  \
glBindBuffer(GL_SHADER_STORAGE_BUFFER, val); \
glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(type), NULL, GL_STATIC_DRAW); \
glEnableVertexAttribArray(attribPtr); \
glVertexAttribPointer(attribPtr, 4, GL_FLOAT, GL_FALSE, sizeof(type), (void*)0) \

#define INIT_BUFFER_BEGIN(size, type) \
{ \
    type* buffer = static_cast<type*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size * sizeof(type), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT)); \
    for (uint32_t i = 0; i < mNumMaxParticles; i++) \
    {
#define INIT_BUFFER_END() \
    } \
} \
glUnmapBuffer(GL_SHADER_STORAGE_BUFFER)

CsParticleSystem::CsParticleSystem(uint32_t maxParticles, uint32_t groupSize)
    : mVao(0)
    , mAtomicBuffer(0)
    , mPosSsbo(0)
    , mVelSsbo(0)
    , mColSsbo(0)
    , mLifeSsbo(0)
    , mIndexSsbo(0)
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
    if (mIndexSsbo != 0)
    {
        glDeleteBuffers(1, &mIndexSsbo);
        mIndexSsbo = 0;
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
    CHECK_GL_ERROR();

    glGenBuffers(1, &mAtomicBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, GetAtomicSize() * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
    ResetGenerateCounter();
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    CREATE_BUFFER(mPosSsbo, mNumMaxParticles, 1, glm::vec4);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec4)
        buffer[i] = glm::vec4(0.0f);
        buffer[i].w = 1.0f;
    INIT_BUFFER_END();

    CREATE_BUFFER(mVelSsbo, mNumMaxParticles, 2, glm::vec4);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec4)
        buffer[i] = glm::vec4(0.0f);
    INIT_BUFFER_END();

    CREATE_BUFFER(mColSsbo, mNumMaxParticles, 3, glm::vec4);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec4)
        buffer[i] = glm::vec4(1.0f);
    INIT_BUFFER_END();

    CREATE_BUFFER(mLifeSsbo, mNumMaxParticles, 4, glm::vec4);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::vec4)
        buffer[i] = glm::vec4(0.0f);
    INIT_BUFFER_END();

#if SORT
    CREATE_BUFFER(mIndexSsbo, mNumMaxParticles, 5, glm::uvec4);
    INIT_BUFFER_BEGIN(mNumMaxParticles, glm::uvec4)
        buffer[i] = glm::uvec4(0.0f);
    INIT_BUFFER_END();
#endif

    CHECK_GL_ERROR();

    std::vector<std::pair<std::string, std::string>> replaceParts;
    replaceParts.emplace_back("LOCAL_SIZE_X", std::to_string(mLocalWorkGroupSize.x));
    replaceParts.emplace_back("DISPATCH_SIZE", std::to_string(GetDispatchSize()));
    replaceParts.emplace_back("ATOMIC_OFFSET1", std::to_string(GetDispatchSize() * sizeof(uint32_t)));

#if SORT
    replaceParts.emplace_back("INDEX_BUFFER_DECL", "layout(std140, binding = 5) buffer Index{ uvec4 Indices[]; };\n");
    replaceParts.emplace_back("INDEX_BUFFER_ID", "uint id = Indices[gl_VertexID].x;\n");
    replaceParts.emplace_back("INDEX_BUFFER_SET_ID", "Indices[index].x = gid;\n");
    replaceParts.emplace_back("SORTED_VERTICES_ID", "");
#else
    replaceParts.emplace_back("INDEX_BUFFER_DECL", "");
    replaceParts.emplace_back("INDEX_BUFFER_ID", "");
    replaceParts.emplace_back("INDEX_BUFFER_SET_ID", "");
    replaceParts.emplace_back("SORTED_VERTICES_ID", "uint id = gl_VertexID;\n");
#endif

    bool success = true;
    success &= mComputeShader.LoadAndCompile("shader/cs_particle/basic.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mComputeShader.AttachLoadedShaders();
    success &= mComputeShader.Link();
    success &= mSortShader.LoadAndCompile("shader/cs_particle/sort.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mSortShader.AttachLoadedShaders();
    success &= mSortShader.Link();
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.gs", Shader::SHADER_TYPE_GEOMETRY);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT, mRenderFsMap);
    success &= mRenderShader.AttachLoadedShaders();
    success &= mRenderShader.Link();

    return success;
}

void CsParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
    CHECK_GL_ERROR();
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBuffer);
    CHECK_GL_ERROR();
    ResetGenerateCounter();
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    CHECK_GL_ERROR();

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, mAtomicBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mPosSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mVelSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mColSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mLifeSsbo);
#if SORT
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mIndexSsbo);
#endif

    CHECK_GL_ERROR();

    float timeForParticle = 1.0f / EmitRate;

    mCurrentGenerateOffset += deltaTime;
    uint32_t numToGenerate = 0;

    // Todo: Could improve
    while (mCurrentGenerateOffset >= timeForParticle)
    {
        numToGenerate++;
        mCurrentGenerateOffset -= timeForParticle;
    }

    CHECK_GL_ERROR();

    mComputeShader.Use();
    mComputeShader.SetFloat("uDeltaTime", deltaTime);
    SetNumToGenerate(numToGenerate);

    CHECK_GL_ERROR();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchCompute(GetDispatchSize(), 1, 1);
    CHECK_GL_ERROR();

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    ReadGeneratedParticles();

#if SORT
    Sort();
#endif

    CHECK_GL_ERROR();
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
    glDrawArrays(GL_POINTS, 0, mNumParticles);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void CsParticleSystem::ReadGeneratedParticles()
{
    auto atomicPtr = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, GetAtomicSize() * sizeof(uint32_t), GL_MAP_READ_BIT));

    mNumParticles = atomicPtr[1];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    CHECK_GL_ERROR();
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
    uint32_t atomicSize = GetAtomicSize();
    auto atomicPtr = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, GetAtomicSize() * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    memset(atomicPtr, 0U, GetAtomicSize() * sizeof(uint32_t));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

void CsParticleSystem::SetNumToGenerate(uint32_t numToGenerate)
{
    if (numToGenerate == 0)
    {
        CHECK_GL_ERROR();
        return;
    }

    auto atomicPtr = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, GetAtomicSize() * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

    atomicPtr[0] = numToGenerate;

    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    CHECK_GL_ERROR();
}

uint32_t CsParticleSystem::GetDispatchSize() const
{
    return mNumMaxParticles / mLocalWorkGroupSize.x;
}

uint32_t CsParticleSystem::GetAtomicSize() const
{
    return 1 * 2;
}