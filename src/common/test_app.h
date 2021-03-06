#include <cstdint>

#include "defines.h"

#include "particle_system/cpu_i_particle_system.h"
#include "particle_system/tf_particle_system.h"

#include "particle_system/cs_particle_system.h"

#include "particle_system/fs_particle_system.h"

#include "gl/camera.h"
#include "gl/texture_2d.h"

class TestApp
{
public:
	TestApp();
	~TestApp();

	void Resize(uint32_t width, uint32_t height);
	bool Init();
	void Step();

	void UpdateParticleSystems(float deltaTime);

	void LateUpdateParticleSystems(uint32_t& particles);

	void DrawParticleSystems();

	void LogDebugFrameTime();

	void LogTestTime(float deltaTime);

	void LogQueryTime(std::chrono::system_clock::time_point& now);

	void ProcessLookInput(float deltaX, float deltaY);
	void Zoom(float zoom);

	bool ShouldClose() const;

private:
	bool ReInit();

	void LogTest(uint32_t& particles);

	std::chrono::time_point<std::chrono::system_clock> mLastFrameTime;
	Camera mCamera;

	uint32_t mNumSystems = 0;
	int32_t mFrameCount = 0;
	float mFrameTime = 0.0f;
	float mTimeSinceStart = 0.0f;

	float mTestTime = 0.0f;
	float mTestStartTime = 15.0f;
	float mTestEndTime = 25.0f;
	float mRealTestTime = 0.0f;
	float mQueryTestTime = 0.0f;
	int32_t mTestFrameCount = 0;
	bool mTestFinished = false;
	uint32_t mTestRuns = 0;
	uint32_t mCurrentTestRun = 0;
	std::vector<float> mTestResults;

	uint32_t mMaxParticles = 0;
	float mEmitRate = 0.0f;

	Texture2D mParticleTex;

#if CPU
	Shader mCpuShader;
	CpuIParticleSystem** mParticleSystems = nullptr;
#endif
#if CS
	CsParticleSystem** mParticleSystems = nullptr;
#endif
#if TF
	TfParticleSystem** mParticleSystems = nullptr;
#endif
#if FS
	FsParticleSystem** mParticleSystems = nullptr;
#endif
};