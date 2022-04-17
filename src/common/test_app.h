#include <cstdint>

#include "defines.h"

#include "particle_system/cpu_i_particle_system.h"
#include "particle_system/cs_particle_system.h"
#include "particle_system/tf_particle_system.h"

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

private:
	std::chrono::time_point<std::chrono::system_clock> mLastFrameTime;
	Camera mCamera;

	int mFrameCount;
	float mFrameTime;

	Texture2D mParticleTex;

#if CPU
	Shader mCpuShader;
	CpuIParticleSystem* mCpuParticleSystem;
#endif
#if CS
	CsParticleSystem* mCsParticleSystem;
#endif
#if TF
	TfParticleSystem* mTfParticleSystem;
#endif
};