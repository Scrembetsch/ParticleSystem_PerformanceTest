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

    bool Init();

    void UpdateParticles(float timeStep, const glm::vec3& cameraPos);
    void PrepareRender(Camera* camera);
    void RenderParticles();
    void LateUpdate();

    bool AddModule(TfIModule* psModule);

    uint32_t GetCurrentParticles() const;

    void SetPosition(const glm::vec3& position);
    glm::vec3 GetPosition() const;

    void SetScale(float scale);
    float GetScale() const;

    void SetMinLifetime(float minLifetime);
    void SetMaxLifetime(float maxLifetime);

    void SetMinStartVelocity(const glm::vec3& minVelocity);
    void SetMaxStartVelocity(const glm::vec3& maxVelocity);

    void SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap);
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

    glm::vec3 mPosition;
    float mScale;

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