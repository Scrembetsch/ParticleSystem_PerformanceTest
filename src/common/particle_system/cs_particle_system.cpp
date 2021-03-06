#include "cs_particle_system.h"
#include "../gl/gl_util.h"

#include <sstream>

#define CREATE_BUFFER(val, size, attribPtr, type) \
glGenBuffers(1, &val);  \
glBindBuffer(GL_ARRAY_BUFFER, val); \
glBufferData(GL_ARRAY_BUFFER, size * sizeof(type), NULL, GL_STATIC_DRAW) \
//glEnableVertexAttribArray(attribPtr); \
//glVertexAttribPointer(attribPtr, 4, GL_FLOAT, GL_FALSE, sizeof(type), (void*)0)

#define INIT_BUFFER_BEGIN(size, type) \
{ \
    type* buffer = static_cast<type*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, size * sizeof(type), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT)); \
    for (uint32_t i = 0; i < mNumMaxParticles; i++) \
    {
#define INIT_BUFFER_END() \
    } \
} \
glUnmapBuffer(GL_ARRAY_BUFFER)

CsParticleSystem::CsParticleSystem(uint32_t maxParticles, uint32_t groupSize)
    : mVao(0)
    , mAtomicBuffer(0)
    , mParticleSsbo(0)
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
    , mPosition(0.0f)
    , mScale(1.0f)
    , mSortTime(0)
{
}

CsParticleSystem::~CsParticleSystem()
{
    if (mAtomicBuffer != 0)
    {
        glDeleteBuffers(1, &mAtomicBuffer);
        mAtomicBuffer = 0;
    }
    if (mParticleSsbo != 0)
    {
        glDeleteBuffers(1, &mParticleSsbo);
        mParticleSsbo = 0;
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

    if (mQuery != 0)
    {
        glDeleteQueries(1, &mQuery);
        mQuery = 0;
    }

    for (size_t i = 0; i < mModules.size(); i++)
    {
        delete mModules[i];
    }
}

bool CsParticleSystem::Init()
{
    mAtomicLocations.emplace_back("NumAlive", mAtomicLocations.size());
    for (size_t i = 0; i < mModules.size(); i++)
    {
        const std::vector<std::string>& atomics = mModules[i]->GetAtomicCounterNames();
        for (size_t j = 0; j < atomics.size(); j++)
        {
            mAtomicLocations.emplace_back(atomics[j], mAtomicLocations.size());
        }
    }

    glGenQueries(1, &mQuery);

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);
    CHECK_GL_ERROR();

    glGenBuffers(1, &mAtomicBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, GetAtomicSize() * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
    {
        auto buffer = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, GetAtomicSize() * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
        memset(buffer, 0U, GetAtomicSize() * sizeof(uint32_t));
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    }
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    struct BufferParticle
    {
        glm::vec4 Data0;
        glm::vec4 Data1;
        glm::vec4 Color;
    };

    CREATE_BUFFER(mParticleSsbo, mNumMaxParticles, 1, BufferParticle);
    INIT_BUFFER_BEGIN(mNumMaxParticles, BufferParticle)
        buffer[i].Data0 = glm::vec4(0.0f);
        buffer[i].Data1 = glm::vec4(0.0f);
        buffer[i].Color = glm::vec4(0.0f);
    INIT_BUFFER_END();

    struct IndexStruct
    {
        uint32_t Index;
        float Distance;
    };

    CREATE_BUFFER(mIndexSsbo, mNumMaxParticles, 2, IndexStruct);
    INIT_BUFFER_BEGIN(mNumMaxParticles, IndexStruct)
        buffer[i].Index = 0;
        buffer[i].Distance = -1.0f;
    INIT_BUFFER_END();

    CHECK_GL_ERROR();

    // Add all atomics
    std::string atomics;
    for (uint32_t i = 0; i < GetAtomicSize(); i++)
    {
        atomics += "layout(binding = 0) uniform atomic_uint ";
        atomics += mAtomicLocations[i].first;
        atomics += ";\n";
    }

    std::string moduleMethods;
    std::string moduleCalls;
    std::string moduleUniforms;

    for (size_t i = 0; i < mModules.size(); i++)
    {
        moduleMethods += mModules[i]->GetModuleMethods();
        moduleCalls += mModules[i]->GetMethodCall();
        moduleUniforms += mModules[i]->GetUniforms();
    }

    std::vector<std::pair<std::string, std::string>> replaceParts;
    replaceParts.emplace_back("LOCAL_SIZE_X", std::to_string(mLocalWorkGroupSize.x));
    replaceParts.emplace_back("DISPATCH_SIZE", std::to_string(GetDispatchSize()));

    replaceParts.emplace_back("MODULE_ATOMIC_COUNTERS", atomics);
    replaceParts.emplace_back("MODULE_UNIFORMS", moduleUniforms);
    replaceParts.emplace_back("MODULE_METHODS", moduleMethods);
    replaceParts.emplace_back("MODULE_CALLS", moduleCalls);

    bool success = true;
    success &= mComputeShader.LoadAndCompile("shader/cs_particle/basic.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mComputeShader.AttachLoadedShaders();
    success &= mComputeShader.Link();
    success &= mSortShader.LoadAndCompile("shader/cs_particle/sort.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mSortShader.AttachLoadedShaders();
    success &= mSortShader.Link();
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.vs", Shader::SHADER_TYPE_VERTEX, replaceParts);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT, mRenderFsMap);
    success &= mRenderShader.AttachLoadedShaders();
    success &= mRenderShader.Link();

    return success;
}

void CsParticleSystem::UpdateParticles(float deltaTime, const glm::vec3& cameraPos)
{
    OPTICK_EVENT();

    mComputeShader.Use();
    mComputeShader.SetVec3("uPosition", mPosition);
    mComputeShader.SetVec3("uCameraPos", cameraPos);
    mComputeShader.SetVec3("uRandomSeed", glm::vec3(mRandom.Rand(-10.0f, 20.0f), mRandom.Rand(-10.0f, 20.0f), mRandom.Rand(-10.0f, 20.0f)));
    mComputeShader.SetVec3("uVelocityMin", mMinStartVelocity);
    mComputeShader.SetVec3("uVelocityRange", mMaxStartVelocity - mMinStartVelocity);

    mComputeShader.SetFloat("uLifeTimeMin", mMinLifetime);
    mComputeShader.SetFloat("uLifeTimeRange", mMaxLifetime - mMinLifetime);
    mComputeShader.SetFloat("uDeltaTime", deltaTime);

    {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBuffer);
        auto buffer = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, GetAtomicSize() * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
        buffer[0] = 0U;
        for (size_t i = 0; i < mModules.size(); i++)
        {
            mModules[i]->ApplyShaderValues(deltaTime, &mComputeShader, buffer);
        }
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }
    CHECK_GL_ERROR();

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, mAtomicBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mParticleSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mIndexSsbo);

    CHECK_GL_ERROR();

    glDispatchCompute(GetDispatchSize(), 1, 1);
    CHECK_GL_ERROR();

#if SORT
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    Sort();
#endif

    CHECK_GL_ERROR();
}

void CsParticleSystem::LateUpdate()
{
    ReadbackAtomicData();
}

void CsParticleSystem::PrepareRender(Camera* camera)
{
    mProjection = glm::perspective(glm::radians(camera->Zoom), camera->ViewWidth / camera->ViewHeight, 0.1f, 200.0f);
    mView = camera->GetViewMatrix();

    mQuad1 = glm::normalize(glm::cross(camera->Front, camera->Up));
    mQuad2 = glm::normalize(glm::cross(camera->Front, mQuad1));
}

bool CsParticleSystem::AddModule(CsIModule* psModule)
{
    mModules.push_back(psModule);
    return true;
}

void CsParticleSystem::RenderParticles()
{
    OPTICK_EVENT();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mParticleSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mIndexSsbo);

    mRenderShader.Use();
    mRenderShader.SetMat4("uProjection", mProjection);
    mRenderShader.SetMat4("uView", mView);

    mRenderShader.SetVec3("uQuad1", mQuad1);
    mRenderShader.SetVec3("uQuad2", mQuad2);

    mRenderShader.SetFloat("uScale", mScale);

    glBindVertexArray(mVao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mNumParticles);
}

// This is very slow and is only needed to read number of particles back (not much performance difference to drawing "more particles")
void CsParticleSystem::ReadbackAtomicData()
{
    //OPTICK_EVENT();

    uint32_t* atomicPtr = static_cast<uint32_t*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, GetAtomicSize() * sizeof(uint32_t), GL_MAP_READ_BIT));
    mNumParticles = atomicPtr[0];
    for (size_t i = 0; i < mModules.size(); i++)
    {
        mModules[i]->ReadbackAtomicData(atomicPtr);
    }
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    CHECK_GL_ERROR();
}

void CsParticleSystem::Sort()
{
    OPTICK_EVENT();

#if MEASURE_SORT_TIME
    glBeginQuery(GL_TIME_ELAPSED, mQuery);
#endif

    mSortShader.Use();
    uint32_t h = mLocalWorkGroupSize.x * 2;

    uint32_t particlesToSort = mNumMaxParticles;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mAtomicBuffer);

    SortLocalBms(particlesToSort, h);

    // we must now double h, as this happens before every flip
    h *= 2;

    for (; h <= particlesToSort; h *= 2) {

        SortBigFlip(particlesToSort, h);

        for (uint32_t hh = h / 2; hh > 1; hh /= 2) {

            if (hh <= mLocalWorkGroupSize.x * 2) {
                SortLocalDisperse(particlesToSort, hh);
                break;
            }
            else {
                SortBigDisperse(particlesToSort, hh);
            }
        }
    }

#if MEASURE_SORT_TIME
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(mQuery, GL_QUERY_RESULT, &mSortTime);
#endif
}

void CsParticleSystem::SortLocalBms(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 0);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::SortLocalDisperse(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 1);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::SortBigFlip(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 2);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void CsParticleSystem::SortBigDisperse(uint32_t n, uint32_t h)
{
    mSortShader.SetUInt("uAlgorithm", 3);
    mSortShader.SetUInt("uN", n);
    mSortShader.SetUInt("uH", h);

    glDispatchCompute(n / (mLocalWorkGroupSize.x * 2), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

uint32_t CsParticleSystem::GetCurrentParticles() const
{
    return mNumParticles;
}

void CsParticleSystem::SetPosition(const glm::vec3& position)
{
    mPosition = position;
}

glm::vec3 CsParticleSystem::GetPosition() const
{
    return mPosition;
}

void CsParticleSystem::SetScale(float scale)
{
    mScale = scale;
}

float CsParticleSystem::GetScale() const
{
    return mScale;
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

uint32_t CsParticleSystem::GetAtomicLocation(const std::string& name) const
{
    const uint32_t atomicSize = GetAtomicSize();
    for (uint32_t i = 0; i < atomicSize; i++)
    {
        if (mAtomicLocations[i].first == name)
        {
            return i;
        }
    }
    return -1;
}

uint32_t CsParticleSystem::GetDispatchSize() const
{
    return mNumMaxParticles / mLocalWorkGroupSize.x;
}

uint32_t CsParticleSystem::GetAtomicSize() const
{
    return static_cast<uint32_t>(mAtomicLocations.size());
}

float CsParticleSystem::GetSortTime() const
{
    return float(mSortTime) / 1000 / 1000 / 1000;
}
