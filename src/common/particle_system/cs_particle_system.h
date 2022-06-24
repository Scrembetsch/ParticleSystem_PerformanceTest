#pragma once
#include "../glm/glm.hpp"
#include "../gl/gl.h"
#include "../gl/shader.h"
#include "../gl/camera.h"
#include "../util/random.h"
#include "../defines.h"

#include "cs_i_module.h"

#include <cstdint>
#include <unordered_map>

class CsParticleSystem
{
public:
	CsParticleSystem(uint32_t maxParticles, uint32_t groupSize);
	virtual ~CsParticleSystem();

	bool Init();

	void UpdateParticles(float deltaTime, const glm::vec3& cameraPos);
	void PrepareRender(Camera* camera);
	void RenderParticles();

	bool AddModule(CsIModule* psModule);

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

	uint32_t GetAtomicLocation(const std::string& name) const;
private:
	void Sort();
	void SortLocalBms(uint32_t n, uint32_t h);
	void SortBigFlip(uint32_t n, uint32_t h);
	void SortLocalDisperse(uint32_t n, uint32_t h);
	void SortBigDisperse(uint32_t n, uint32_t h);

	uint32_t GetDispatchSize() const;
	uint32_t GetAtomicSize() const;

	void ReadbackAtomicData();

	uint32_t mVao;

	uint32_t mAtomicBuffer;
	uint32_t mPosSsbo;
	uint32_t mVelSsbo;
	uint32_t mColSsbo;
	uint32_t mLifeSsbo;
	uint32_t mIndexSsbo;

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

	glm::vec3 mPosition;
	float mScale;

	Shader mComputeShader;
	Shader mSortShader;
	Shader mRenderShader;

	std::vector<std::pair<std::string, std::string>> mRenderFsMap;

	std::vector<CsIModule*> mModules;
	std::vector<std::pair<std::string, uint32_t>> mAtomicLocations;

	float mCurrentGenerateOffset;
};