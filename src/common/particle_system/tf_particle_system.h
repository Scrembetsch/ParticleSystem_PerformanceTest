#pragma once

#include "../gl/shader.h"
#include "../gl/camera.h"
#include "../gl/texture_2d.h"
#include "../util/random.h"
#include "../defines.h"

#include "tf_particle.h"
#include "tf_i_module.h"

class TfParticleSystem
{
private:
public:
    TfParticleSystem(uint32_t maxParticles);
    virtual ~TfParticleSystem();

    virtual bool Init();

    virtual void UpdateParticles(float timeStep, const glm::vec3& cameraPos);
    void PrepareRender(Camera* camera);
    void RenderParticles();

    virtual bool AddModule(TfIModule* psModule);

    virtual uint32_t GetCurrentParticles() const;

    virtual void SetMinLifetime(float minLifetime);
    virtual void SetMaxLifetime(float maxLifetime);

    virtual void SetMinStartVelocity(const glm::vec3& minVelocity);
    virtual void SetMaxStartVelocity(const glm::vec3& maxVelocity);

    virtual void SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap);
    Shader* GetRenderShader();

    int32_t GetMaxVerticesPerEmitter() const;
    uint32_t GetEmitters() const;

protected:
    static const uint32_t sBufferSize = 2U;

    uint32_t mTransformFeedbackBuffer;

    uint32_t mVbos[sBufferSize];
    uint32_t mVaos[sBufferSize];

    uint32_t mQuery;

    uint32_t mCurrentReadBuffer;
    uint32_t mCurrentWriteBuffer;

    uint32_t mNumMaxParticles;
    uint32_t mNumParticles;

    int32_t mMaxVertices;
    uint32_t mNumEmitters;

    glm::mat4 mProjection;
    glm::mat4 mView;
    glm::vec3 mQuad1;
    glm::vec3 mQuad2;

    Random mRandom;

    float mMinLifetime;
    float mMaxLifetime;

    glm::vec3 mMinStartVelocity;
    glm::vec3 mMaxStartVelocity;

    Shader mRenderShader;
    Shader mUpdateShader;

    std::vector<TfIModule*> mModules;
    std::vector<std::pair<std::string, std::string>> mRenderFsMap;

    uint32_t mSortBuffer;
    Texture2D mIndexTex[2];
    Shader mSortShader;
    Shader mPrepSortShader;

    uint32_t mUpdateVao;
    uint32_t mUpdateVbo;
    uint32_t mResolutionX;
    uint32_t mResolutionY;
    uint32_t mSortCurrentReadBuffer;
    uint32_t mSortCurrentWriteBuffer;

    void Sort();
    void PrepSort();
};