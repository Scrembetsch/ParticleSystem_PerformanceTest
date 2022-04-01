#include "cs_particle_system.h"

#include <sstream>

CsParticleSystem::CsParticleSystem()
    : mPosSsbo(0)
    , mVelSsbo(0)
    , mColSsbo(0)
    , mVao(0)
    , mLocalWorkGroupSize(cGroupSize, 1, 1)
{
}

CsParticleSystem::~CsParticleSystem()
{
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
    if (mVao != 0)
    {
        glDeleteVertexArrays(1, &mVao);
        mVao = 0;
    }
}

bool CsParticleSystem::Init()
{
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

    glGenVertexArrays(1, &mVao);
    glGenBuffers(1, &mPosSsbo);
    glBindVertexArray(mVao);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mPosSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cMaxParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

    glm::vec4* points = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, cMaxParticles * sizeof(glm::vec4), bufMask));
    for (int i = 0; i < cMaxParticles; i++)
    {
        points[i].x = 0.0f;
        points[i].y = 0.0f;
        points[i].z = 0.0f;
        points[i].w = 1.0f;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &mVelSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mVelSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cMaxParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);

    glm::vec4* vels = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, cMaxParticles * sizeof(glm::vec4), bufMask));
    for (int i = 0; i < cMaxParticles; i++)
    {
        vels[i].x = sin(mRng.Rand01() * 3.14f);
        vels[i].y = cos(mRng.Rand01() * 3.14f);
        vels[i].z = sin(mRng.Rand01() * 3.14f);
        vels[i].w = 1.0f;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &mColSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mColSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cMaxParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);

    glm::vec4* cols = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, cMaxParticles * sizeof(glm::vec4), bufMask));
    for (int i = 0; i < cMaxParticles; i++)
    {
        cols[i].x = mRng.Rand01();
        cols[i].y = mRng.Rand01();
        cols[i].z = mRng.Rand01();
        cols[i].w = 1.0f;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    std::stringstream ss;
    ss << "local_size_x = " << mLocalWorkGroupSize.x << ", ";
    ss << "local_size_y = " << mLocalWorkGroupSize.y << ", ";
    ss << "local_size_z = " << mLocalWorkGroupSize.z;

    std::vector<std::pair<std::string, std::string>> replaceParts;
    replaceParts.emplace_back(std::pair<std::string, std::string>("LOCAL_WORK_GROUP_SIZE", ss.str()));

    bool success = true;

    success &= mComputeShader.LoadAndCompile("shader/cs_particle/basic.cs", Shader::SHADER_TYPE_COMPUTE, replaceParts);
    success &= mComputeShader.AttachLoadedShaders();
    success &= mComputeShader.Link();
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.vs", Shader::SHADER_TYPE_VERTEX);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.gs", Shader::SHADER_TYPE_GEOMETRY);
    success &= mRenderShader.LoadAndCompile("shader/cs_particle/render.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mRenderShader.AttachLoadedShaders();
    success &= mRenderShader.Link();

    return success;
}

void CsParticleSystem::Update(float dt)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mPosSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mVelSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, mColSsbo);

    mComputeShader.Use();
    mComputeShader.SetFloat("uDeltaTime", dt);
    glDispatchCompute(cMaxParticles / mLocalWorkGroupSize.x, 1, 1);

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    //GLint bufMask = GL_MAP_READ_BIT;

    //glm::vec4* points = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, cMaxParticles * sizeof(glm::vec4), bufMask));
    //for (int i = 0; i < 5; i++)
    //{
    //    glm::vec4 newPoint = mProj * mView * points[i];
    //    LOG("CS", "%g, %g, %g", newPoint.x, newPoint.y, newPoint.z);
    //}
    //glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void CsParticleSystem::PrepareRender(const glm::mat4& projMat, const glm::mat4& viewMat, const glm::vec3& up, const glm::vec3& front)
{
    mProj = projMat;
    mView = viewMat;
    mUp = up;
    mFront = front;

    mQuad1 = glm::cross(front, up);
    mQuad1 = glm::normalize(mQuad1);
    mQuad2 = glm::cross(front, mQuad1);
    mQuad2 = glm::normalize(mQuad2);
}

void CsParticleSystem::Render()
{
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    mRenderShader.Use();
    mRenderShader.SetMat4("uProjection", mProj);
    mRenderShader.SetMat4("uView", mView);
    mRenderShader.SetVec3("uQuad1", mQuad1);
    mRenderShader.SetVec3("uQuad2", mQuad2);

    glBindVertexArray(mVao);
    glBindBuffer(GL_ARRAY_BUFFER, mPosSsbo);
    glDrawArrays(GL_POINTS, 0, cMaxParticles);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
