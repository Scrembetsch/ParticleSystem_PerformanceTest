#include <cstdint>

#include "defines.h"

#include "particle_system/cpu_i_particle_system.h"
#include "particle_system/tf_particle_system.h"

#include "particle_system/cs_particle_system_struct.h"
#include "particle_system/cs_particle_system.h"

#include "particle_system/fs_particle_system.h"

#include "gl/camera.h"
#include "gl/texture_2d.h"

class TestApp
{
public:
	TestApp();
	~TestApp() = default;

	void Resize(uint32_t width, uint32_t height);
	bool Init();
	void Step();

	void ProcessLookInput(float deltaX, float deltaY);
	void Zoom(float zoom);

	bool ShouldClose() const;

private:
	bool ReInit();

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
	int32_t mTestFrameCount = 0;
	bool mTestFinished = false;
	uint32_t mTestRuns = 0;
	uint32_t mCurrentTestRun = 0;

	uint32_t mMaxParticles = 0;
	float mEmitRate = 0.0f;

	Texture2D mParticleTex;

#if CPU
	Shader mCpuShader;
	CpuIParticleSystem** mParticleSystems = nullptr;
#endif
#if CS
	#if USE_STRUCT
		CsParticleSystemStruct** mParticleSystems = nullptr;
	#else
		CsParticleSystem** mParticleSystems = nullptr;
	#endif
#endif
#if TF
	TfParticleSystem** mParticleSystems = nullptr;
#endif
#if FS
	FsParticleSystem** mParticleSystems = nullptr;
#endif
};