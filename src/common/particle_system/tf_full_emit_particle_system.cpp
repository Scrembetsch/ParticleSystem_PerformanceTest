#include "tf_full_emit_particle_system.h"

TfFullEmitParticleSystem::TfFullEmitParticleSystem(uint32_t maxParticles, float emitRate)
    : TfParticleSystem(maxParticles)
    , EmitRate(emitRate)
    , mCurrentGenerateOffset(0)
{
    glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &mMaxComponents);
    mComponentSize = sizeof(TfParticle) / sizeof(float);

    mMaxVerticesPerPass = std::min((mMaxComponents / mComponentSize), mMaxVertices);
}

bool TfFullEmitParticleSystem::Init()
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
    replaceMapGs.emplace_back("MAX_OUTPUT_VERTICES", std::to_string(mMaxVertices));

    replaceMapVs.emplace_back("MODULE_CALLS", callsVs);
    replaceMapGs.emplace_back("MODULE_CALLS", callsGs);

    std::vector<std::pair<std::string, std::string>> replaceParts;
    replaceParts.emplace_back("LOCAL_SIZE_X", std::to_string(mLocalWorkGroupSize.x));

    success &= mEmitShader.LoadAndCompile("shader/tf_particle/update_emit.vs", Shader::SHADER_TYPE_VERTEX, replaceMapVs);
    success &= mEmitShader.LoadAndCompile("shader/tf_particle/update_emit.gs", Shader::SHADER_TYPE_GEOMETRY, replaceMapGs);
    success &= mEmitShader.LoadAndCompile("shader/tf_particle/update_emit.fs", Shader::SHADER_TYPE_FRAGMENT);
    success &= mEmitShader.AttachLoadedShaders();

    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update_without_emit.vs", Shader::SHADER_TYPE_VERTEX, replaceMapVs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update_without_emit.gs", Shader::SHADER_TYPE_GEOMETRY, replaceMapGs);
    success &= mUpdateShader.LoadAndCompile("shader/tf_particle/update_without_emit.fs", Shader::SHADER_TYPE_FRAGMENT);
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
    glTransformFeedbackVaryings(mEmitShader.GetId(), varyingSize, sVaryings, GL_INTERLEAVED_ATTRIBS);

    if (!mUpdateShader.Link())
    {
        return false;
    }
    if (!mEmitShader.Link())
    {
        return false;
    }

    glGenTransformFeedbacks(1, &mTransformFeedbackBuffer);
    glGenQueries(1, &mQuery);

    glGenBuffers(sBufferSize, mVbos);
    glGenVertexArrays(sBufferSize, mVaos);

    TfParticle initParticle;
    initParticle.Type = 0.0f;

    for (uint32_t i = 0; i < sBufferSize; i++)
    {
        glBindVertexArray(mVaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TfParticle) * mNumMaxParticles, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TfParticle), &initParticle);

        for (uint32_t j = 0; j < varyingSize; j++)
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

void TfFullEmitParticleSystem::UpdateParticles(float timeStep, const glm::vec3& cameraPos)
{
    float timeForOneParticle = 1.0f / EmitRate;

    mCurrentGenerateOffset += timeStep;
    uint32_t numToGenerate = 0;

    // Todo: Could improve
    while (mCurrentGenerateOffset >= timeForOneParticle)
    {
        numToGenerate++;
        mCurrentGenerateOffset -= timeForOneParticle;
    }
    CHECK_GL_ERROR();

    mEmitShader.Use();
    CHECK_GL_ERROR();

    mEmitShader.SetVec3("uPosition", glm::vec3(0.0f));
    mEmitShader.SetVec3("uVelocityMin", mMinStartVelocity);
    mEmitShader.SetVec3("uVelocityRange", mMaxStartVelocity - mMinStartVelocity);

    mEmitShader.SetFloat("uTimeStep", timeStep);
    mEmitShader.SetFloat("uLifeTimeMin", mMinLifetime);
    mEmitShader.SetFloat("uLifeTimeRange", mMaxLifetime - mMinLifetime);
    CHECK_GL_ERROR();

    while (numToGenerate > 0)
    {
        glm::vec3 randomSeed = glm::vec3(mRandom.Rand(-10.0f, 20.0f), mRandom.Rand(-10.0f, 20.0f), mRandom.Rand(-10.0f, 20.0f));
        mEmitShader.SetVec3("uRandomSeed", randomSeed);
        mEmitShader.SetUInt("uNumToGenerate", numToGenerate);
        CHECK_GL_ERROR();

        if (numToGenerate < mMaxVerticesPerPass)
        {
            numToGenerate = 0;
        }
        else
        {
            numToGenerate -= mMaxVerticesPerPass;
        }

        CHECK_GL_ERROR();

        glEnable(GL_RASTERIZER_DISCARD);
        CHECK_GL_ERROR();

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbackBuffer);
        CHECK_GL_ERROR();

        glBindVertexArray(mVaos[mCurrentReadBuffer]);
        CHECK_GL_ERROR();

        // 1 -  use other buffer
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVbos[1 - mCurrentReadBuffer]);
        CHECK_GL_ERROR();

        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mQuery);
        CHECK_GL_ERROR();

        glBeginTransformFeedback(GL_POINTS);
        CHECK_GL_ERROR();

        glDrawArrays(GL_POINTS, 0, mNumParticles);
        CHECK_GL_ERROR();

        glEndTransformFeedback();
        CHECK_GL_ERROR();

        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
        CHECK_GL_ERROR();

        glGetQueryObjectuiv(mQuery, GL_QUERY_RESULT, &mNumParticles);

        // toggle buffer
        mCurrentReadBuffer = 1 - mCurrentReadBuffer;

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glDisable(GL_RASTERIZER_DISCARD);

        CHECK_GL_ERROR();
    }

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

    Sort();
}