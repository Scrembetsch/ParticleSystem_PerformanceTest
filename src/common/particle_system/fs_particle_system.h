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
	~FsParticleSystem();

	bool Init();

	void UpdateParticles(float deltaTime, const glm::vec3& cameraPos);
	void PrepareRender(Camera* camera);
	void RenderParticles();

	void Emit(uint32_t numToGenerate);

	void AddModule(FsIModule* mod);

	uint32_t GetCurrentParticles() const;

	void SetMinLifetime(float minLifetime);
	void SetMaxLifetime(float maxLifetime);

	void SetMinStartVelocity(const glm::vec3& minVelocity);
	void SetMaxStartVelocity(const glm::vec3& maxVelocity);

	void SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap);
	Shader* GetRenderShader();

private:
	void CheckForDeadParticles();

	// Sorting
#if SORT
	void Sort();
	uint32_t mSortBuffer = 0;
	Shader mSortShader;
#endif

	uint32_t mEmptyVao;
	uint32_t mEmptyVbo;
	uint32_t mUpdateVao;
	uint32_t mUpdateVbo;

	uint32_t mFramebuffer[2] = {0};
	Texture2D mPosition[2];
	Texture2D mVelocity[2];
	Texture2D mColor[2];
	Texture2D mIndex[2];
	Texture2D mData0;

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

	Shader mUpdateShader;
	Shader mRenderShader;

	std::vector<std::pair<std::string, std::string>> mRenderFsMap;

	float mCurrentTime;
	std::queue<uint32_t> mGenerateQueue;
	glm::vec4* mData0Array;

	std::vector<FsIModule*> mModules;
};