#pragma once
#include "../glm/glm.hpp"
#include "../gl/gl.h"
#include "../gl/shader.h"
#include "../gl/camera.h"
#include "../gl/texture_2d.h"
#include "../util/random.h"
#include "../defines.h"

#include "fs_i_module.h"

#include <cstdint>
#include <stack>
#include <queue>

class FsParticleSystem
{
public:
	FsParticleSystem(uint32_t maxParticles);
	virtual ~FsParticleSystem();

	bool Init();

	void UpdateParticles(float deltaTime, const glm::vec3& cameraPos);
	void PrepareRender(Camera* camera);
	void RenderParticles();
	void LateUpdate();

	void Emit(uint32_t numToGenerate);

	void AddModule(FsIModule* mod);

	uint32_t GetCurrentParticles() const;

	void SetMinLifetime(float minLifetime);
	void SetMaxLifetime(float maxLifetime);

	void SetMinStartVelocity(const glm::vec3& minVelocity);
	void SetMaxStartVelocity(const glm::vec3& maxVelocity);

	void SetPosition(const glm::vec3& position);
	glm::vec3 GetPosition() const;

	void SetScale(float scale);
	float GetScale() const;

	void SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap);
	Shader* GetRenderShader();

	float GetSortTime() const;

private:
	void CheckForDeadParticles();

	// Sorting
#if SORT
	void Sort();
	uint32_t mSortBuffer = 0;
	Shader mSortShader;

	uint32_t mQuery;
#endif

	uint32_t mSortTime;

	uint32_t mEmptyVao;
	uint32_t mEmptyVbo;
	uint32_t mUpdateVao;
	uint32_t mUpdateVbo;

	uint32_t mFramebuffer[2] = {0};
	Texture2D mPositionTex[2];
	Texture2D mVelocityTex[2];
	Texture2D mColorTex[2];
	Texture2D mIndexTex[2];
	Texture2D mData0Tex;

	uint32_t mResolutionX;
	uint32_t mResolutionY;
	uint32_t mNumMaxParticles;
	uint32_t mNumParticles;

	glm::mat4 mProjection;
	glm::mat4 mView;
	glm::vec3 mQuad1;
	glm::vec3 mQuad2;

	Random mRandom;

	float mMinLifetime;
	float mMaxLifetime;

	uint32_t mCurrentReadBuffer;
	uint32_t mCurrentWriteBuffer;

	uint32_t mSortCurrentReadBuffer;
	uint32_t mSortCurrentWriteBuffer;

	glm::vec3 mMinStartVelocity;
	glm::vec3 mMaxStartVelocity;

	glm::vec3 mPosition;
	float mScale;

	Shader mUpdateShader;
	Shader mRenderShader;

	std::vector<std::pair<std::string, std::string>> mRenderFsMap;

	float mCurrentTime;
	std::queue<uint32_t> mGenerateQueue;
	glm::vec4* mData0Array;

	std::vector<FsIModule*> mModules;
};