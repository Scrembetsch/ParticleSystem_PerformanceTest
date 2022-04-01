#pragma once

#include "../gl/shader.h"
#include "../util/random.h"

class TfParticleSystem
{
private:
    struct Particle
    {
        glm::vec3 Position;
        glm::vec3 Velocity;
        glm::vec3 Color;
        float LifeTime;
        float Size;
        float Type;
    };
public:
    TfParticleSystem();
    ~TfParticleSystem();

    bool Init();

    void UpdateParticles(float timeStep);
    void RenderParticles();

    void SetGeneratorProperties(const glm::vec3& position, const glm::vec3& velocityMin, const glm::vec3& velocityMax, const glm::vec3& gravity, const glm::vec3 color, float minLifeTime, float maxLifeTime, float size, float spawnTime, int numToGenerate);
    void SetGeneratorPosition(const glm::vec3& position);
    uint32_t GetNumParticles() const;

    void SetMatrices(const glm::mat4& projection, const glm::mat4& viewMat, const glm::vec3& view, const glm::vec3& upVector);

    float mNextGenerationTime;

    Shader mRenderShader;
    Shader mUpdateShader;
private:
    static const uint32_t sBufferSize = 2;
    static const uint32_t sMaxParticles = 100000;

    uint32_t mTransformFeedbackBuffer;

    uint32_t mVbos[sBufferSize];
    uint32_t mVaos[sBufferSize];

    uint32_t mQuery;

    uint32_t mCurrentReadBuffer;
    uint32_t mNumParticles;

    glm::mat4 mProjection;
    glm::mat4 mView;
    glm::vec3 mQuad1;
    glm::vec3 mQuad2;

    float mElapsedTime;

    glm::vec3 mPosition;
    glm::vec3 mVelocityMin;
    glm::vec3 mVelocityRange;
    glm::vec3 mGravity;
    glm::vec3 mColor;

    float mLifeTimeMin;
    float mLifeTimeRange;
    float mSize;

    int mNumToGenerate;

    Random mRng;
};