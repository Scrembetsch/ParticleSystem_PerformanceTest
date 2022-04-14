#pragma once
#include "../glm/glm.hpp"
#include "../gl/gl.h"
#include "../gl/shader.h"
#include "../gl/camera.h"
#include "../util/random.h"

#include <cstdint>

class CsParticleSystem
{
public:
	CsParticleSystem(uint32_t maxParticles, uint32_t groupSize);
	~CsParticleSystem();

	bool Init();

	void UpdateParticles(float deltaTime, const glm::vec3& cameraPos);
	void PrepareRender(Camera* camera);
	void RenderParticles();

	virtual uint32_t GetCurrentParticles() const;

	virtual void SetMinLifetime(float minLifetime);
	virtual void SetMaxLifetime(float maxLifetime);

	virtual void SetMinStartVelocity(const glm::vec3& minVelocity);
	virtual void SetMaxStartVelocity(const glm::vec3& maxVelocity);

	virtual void SetRenderFragReplaceMap(const std::vector<std::pair<std::string, std::string>>& replaceMap);
	Shader* GetRenderShader();

	float EmitRate;

private:
	void ResetGenerateCounter();
	void Sort();
	void SortLocalBms(uint32_t n, uint32_t h);
	void SortBigFlip(uint32_t n, uint32_t h);
	void SortLocalDisperse(uint32_t n, uint32_t h);
	void SortBigDisperse(uint32_t n, uint32_t h);

	uint32_t mVao;

	uint32_t mAtomicBuffer;
	uint32_t mPosSsbo;
	uint32_t mVelSsbo;
	uint32_t mColSsbo;
	uint32_t mLifeSsbo;

	uint32_t mNumMaxParticles;
	uint32_t mNumParticles;

	glm::uvec3 mLocalWorkGroupSize;

	glm::mat4 mProjection;
	glm::mat4 mView;
	glm::vec3 mQuad1;
	glm::vec3 mQuad2;

	Random mRandom;

	float mMinLifetime;
	float mMaxLifetime;

	glm::vec3 mMinStartVelocity;
	glm::vec3 mMaxStartVelocity;

	Shader mComputeShader;
	Shader mSortShader;
	Shader mRenderShader;

	std::vector<std::pair<std::string, std::string>> mRenderFsMap;

	float mCurrentGenerateOffset;
};