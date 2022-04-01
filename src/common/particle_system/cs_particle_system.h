#pragma once
#include "../glm/glm.hpp"
#include "../gl/gl.h"
#include "../gl/shader.h"
#include "../util/random.h"

#include <cstdint>

class CsParticleSystem
{
private:
	const uint32_t cMaxParticles = 1028 * 1028;
	const uint32_t cGroupSize = 128;

public:
	CsParticleSystem();
	~CsParticleSystem();

	bool Init();

	void Update(float dt);

	void PrepareRender(const glm::mat4& projMat, const glm::mat4& viewMat, const glm::vec3& up, const glm::vec3& front);
	void Render();

private:
	GLuint mPosSsbo;
	GLuint mVelSsbo;
	GLuint mColSsbo;

	GLuint mVao;

	Shader mComputeShader;
	Shader mRenderShader;

	glm::mat4 mProj;
	glm::mat4 mView;

	glm::vec3 mFront;
	glm::vec3 mUp;

	glm::vec3 mQuad1;
	glm::vec3 mQuad2;

	glm::vec3 mLocalWorkGroupSize;

	Random mRng;
};