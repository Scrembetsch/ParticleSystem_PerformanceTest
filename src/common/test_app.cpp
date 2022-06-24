#include "test_app.h"

#include "gl/gl.h"
#include "gl/gl_util.h"

#include "particle_system/cpu_serial_particle_system.h"
#include "particle_system/cpu_serial_instance_particle_system.h"
#include "particle_system/cpu_parallel_particle_system.h"
#include "particle_system/cpu_parallel_instance_particle_system.h"

#include "particle_system/cpu_module_emission.h"
#include "particle_system/cpu_module_velocity_over_lifetime.h"
#include "particle_system/cpu_module_color_over_lifetime.h"
#include "particle_system/cpu_module_noise_position.h"

#include "particle_system/tf_module_emission.h"
#include "particle_system/tf_module_velocity_over_lifetime.h"
#include "particle_system/tf_module_color_over_lifetime.h"

#include "particle_system/fs_module_emission.h"
#include "particle_system/fs_module_color_over_lifetime.h"
#include "particle_system/fs_module_velocity_over_lifetime.h"

#include "particle_system/cs_module_emission.h"
#include "particle_system/cs_module_velocity_over_lifetime.h"
#include "particle_system/cs_module_color_over_lifetime.h"

TestApp::TestApp()
{
	OPTICK_SET_MEMORY_ALLOCATOR(
		[](size_t size) -> void* { return operator new(size); },
		[](void* p) { operator delete(p); },
		[]() { /* Do some TLS initialization here if needed */ }
	);

	OPTICK_THREAD("MainThread");
}

void TestApp::Resize(uint32_t width, uint32_t height)
{
	mCamera.ViewWidth = static_cast<float>(width);
	mCamera.ViewHeight = static_cast<float>(height);
	glViewport(0, 0, width, height);
}

bool TestApp::ReInit()
{
	for (uint32_t i = 0; i < mNumSystems; i++)
		delete mParticleSystems[i];

	delete[] mParticleSystems;

	mNumSystems = 1;
	//uint32_t testRuns[] = { 10, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000, 1500000, 2000000 };
	//uint32_t testRuns[] = { 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7, 1<<8, 1<<9, 1<<10, 1<<11, 1<<12, 1<<13, 1<<14, 1<<15, 1<<16, 1<<17, 1<<18, 1<<19, 1<<20, 1<<21 };
	//uint32_t testRuns[] = { 2500000 };
	////uint32_t testRuns[] = { 20 };
	//uint32_t testRuns[] = { 32 * 32};
	uint32_t testRuns[] = { 512 * 512 };
	mTestRuns = sizeof(testRuns) / sizeof(uint32_t);

	float emitMulti = 5.0f;
	mMaxParticles = testRuns[mCurrentTestRun];
	mEmitRate = mMaxParticles / emitMulti;
	//mEmitRate = 1;
	emitMulti -= emitMulti / 10.0f;

	std::vector<std::pair<std::string, std::string>> replaceMap;
#ifdef _WIN32
	replaceMap.emplace_back("DECL_TEX0", "layout (binding = 0) uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#else
	replaceMap.emplace_back("DECL_TEX0", "uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#endif

	using PSystem =
#if CPU
		CpuIParticleSystem;
#endif
#if CS
		CsParticleSystem;
#endif
#if TF
		TfParticleSystem;
#endif
#if FS
		FsParticleSystem;
#endif

	mParticleSystems = new PSystem*[mNumSystems];
	bool success = true;

	for (uint32_t i = 0; i < mNumSystems; i++)
	{
		PSystem*& mParticleSystem = mParticleSystems[i];

#if CPU
		mParticleSystem = new
#if PARALLEL
#if INSTANCE
			CpuParallelInstanceParticleSystem
#else
			CpuParallelParticleSystem
#endif
#else
#if INSTANCE
			CpuSerialInstanceParticleSystem
#else
			CpuSerialParticleSystem
#endif
#endif
			(mMaxParticles
#if PARALLEL
				, NUM_CPU_THREADS
#endif
				);
		success &= mParticleSystem->Init();

		mParticleSystem->SetMinLifetime(emitMulti);
		mParticleSystem->SetMaxLifetime(emitMulti);
		mParticleSystem->SetMinStartVelocity(glm::vec3(-0.5f, -0.5f, -1.0f));
		mParticleSystem->SetMaxStartVelocity(glm::vec3(0.5f, 0.5f, 0.0f));
		mParticleSystem->AddModule(new CpuModuleEmission(mParticleSystem, mEmitRate));
		mParticleSystem->AddModule(new CpuModuleVelOverLife(mParticleSystem, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.5f, 0.0f, 0.0f)));
		mParticleSystem->AddModule(new CpuModuleColorOverLife(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
#endif
#if TF
		mParticleSystem = new TfParticleSystem(mMaxParticles);

		mParticleSystem->SetMinLifetime(emitMulti);
		mParticleSystem->SetMaxLifetime(emitMulti);
		mParticleSystem->SetRenderFragReplaceMap(replaceMap);
		mParticleSystem->SetMinStartVelocity(glm::vec3(-0.5f, -0.5f, -1.0f));
		mParticleSystem->SetMaxStartVelocity(glm::vec3(0.5f, 0.5f, 0.0f));
		mParticleSystem->AddModule(new TfModuleEmission(mParticleSystem, mEmitRate));
		mParticleSystem->AddModule(new TfModuleVelOverLife(mParticleSystem, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.5f, 0.0f, 0.0f)));
		mParticleSystem->AddModule(new TfModuleColorOverLife(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		success &= mParticleSystem->Init();
#endif
#if FS
		mParticleSystem = new FsParticleSystem(mMaxParticles);

		mParticleSystem->SetMinLifetime(emitMulti);
		mParticleSystem->SetMaxLifetime(emitMulti);
		mParticleSystem->SetRenderFragReplaceMap(replaceMap);
		mParticleSystem->SetMinStartVelocity(glm::vec3(-0.5f, -0.5f, -1.0f));
		mParticleSystem->SetMaxStartVelocity(glm::vec3(0.5f, 0.5f, 0.0f));
		mParticleSystem->AddModule(new FsModuleEmission(mParticleSystem, mEmitRate));
		mParticleSystem->AddModule(new FsModuleVelocityOverLifetime(mParticleSystem, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.5f, 0.0f, 0.0f)));
		mParticleSystem->AddModule(new FsModuleColorOverLifetime(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		success &= mParticleSystem->Init();
#endif
#if CS
		mParticleSystem = new CsParticleSystem(mMaxParticles, (mMaxParticles > WORK_GROUP_SIZE) ? WORK_GROUP_SIZE : mMaxParticles);

		mParticleSystem->SetMinLifetime(emitMulti);
		mParticleSystem->SetMaxLifetime(emitMulti);
		mParticleSystem->SetRenderFragReplaceMap(replaceMap);
		mParticleSystem->SetMinStartVelocity(glm::vec3(-0.5f, -0.5f, -1.0f));
		mParticleSystem->SetMaxStartVelocity(glm::vec3(0.5f, 0.5f, 0.0f));
		mParticleSystem->AddModule(new CsModuleEmission(mParticleSystem, mEmitRate));
		mParticleSystem->AddModule(new CsModuleVelOverLife(mParticleSystem, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.5f, 0.0f, 0.0f)));
		mParticleSystem->AddModule(new CsModuleColorOverLife(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		success &= mParticleSystem->Init();
#endif
		mParticleSystem->SetScale(0.1f);
	}
	mFrameCount = 0;
	mFrameTime = 0.0f;
	mTimeSinceStart = 0.0f;

	mTestTime = 0.0f;
	mTestStartTime = 15.0f;
	mTestEndTime = 25.0f;
	mRealTestTime = 0.0f;
	mTestFrameCount = 0;
	mTestFinished = false;

	return success;
}

bool TestApp::Init()
{
	mParticleTex.mTex = GlUtil::LoadTexture("textures/particle.png");
	mParticleTex.mTexLocation = GL_TEXTURE0;
	mParticleTex.mTexName = "uDiffuseMap";

	bool success = ReInit();

	std::vector<std::pair<std::string, std::string>> replaceMap;
#ifdef _WIN32
	replaceMap.emplace_back("DECL_TEX0", "layout (binding = 0) uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#else
	replaceMap.emplace_back("DECL_TEX0", "uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#endif

#if CPU
#if INSTANCE
	success &= mCpuShader.LoadAndCompile("shader/instance.vs", Shader::ShaderType::SHADER_TYPE_VERTEX);
	success &= mCpuShader.LoadAndCompile("shader/instance.fs", Shader::ShaderType::SHADER_TYPE_FRAGMENT, replaceMap);
#else
	success &= mCpuShader.LoadAndCompile("shader/base.vs", Shader::ShaderType::SHADER_TYPE_VERTEX);
	success &= mCpuShader.LoadAndCompile("shader/base.fs", Shader::ShaderType::SHADER_TYPE_FRAGMENT, replaceMap);
#endif
	success &= mCpuShader.AttachLoadedShaders();
	success &= mCpuShader.Link();
#endif

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mCamera.Position = glm::vec3(0.0f, 0.0f, -80.0f);
	mCamera.Yaw = 90.0f;
	mCamera.UpdateCameraVectors();

	mLastFrameTime = std::chrono::system_clock::now();
	return success;
}

void TestApp::Step()
{
	OPTICK_FRAME("Frame");
	auto now = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsedSeconds = now - mLastFrameTime;
	float deltaTime = elapsedSeconds.count();
	mTimeSinceStart += deltaTime;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uint32_t particles = 0;

	for (uint32_t i = 0; i < mNumSystems; i++)
	{
		glm::vec3 position(0.0f);
		position.x = std::sin(mTimeSinceStart * 3.14f * 0.75f) * 25.0f;
		position.y = std::sin(mTimeSinceStart * 3.14f * 2.0f) * 15.0f;

		position.x += 10.0f;
		position.y -= 10.0f;
		mParticleSystems[i]->SetPosition(position);
#if CPU
		mParticleSystems[i]->PrepareRender(&mCamera);
		mParticleSystems[i]->UpdateParticles(deltaTime, mCamera.Position);
		glm::mat4 projection = glm::perspective(glm::radians(mCamera.Zoom), mCamera.ViewWidth / mCamera.ViewHeight, 0.1f, 200.0f);
		glm::mat4 view = mCamera.GetViewMatrix();

		mCpuShader.Use();
		mCpuShader.SetMat4("uProjection", projection);
		mCpuShader.SetMat4("uView", view);
		mParticleTex.Use(&mCpuShader);
		mParticleSystems[i]->RenderParticles();
		particles = mParticleSystems[i]->GetCurrentParticles();
		CHECK_GL_ERROR();
#endif
#if CS || TF || FS
		mParticleSystems[i]->UpdateParticles(deltaTime, mCamera.Position);

		mParticleSystems[i]->GetRenderShader()->Use();
		mParticleTex.Use(mParticleSystems[i]->GetRenderShader());
		mParticleSystems[i]->PrepareRender(&mCamera);
		CHECK_GL_ERROR();
#endif
		mParticleSystems[i]->RenderParticles();
		CHECK_GL_ERROR();
		particles += mParticleSystems[i]->GetCurrentParticles();
		CHECK_GL_ERROR();
	}

	mFrameTime += deltaTime;
	mFrameCount++;
	if (mFrameTime > 0.5f)
	{
		int fps = static_cast<int>(round(static_cast<float>(mFrameCount) / mFrameTime));
#if _DEBUG
		LOG("FRAME_TIME", "Fps: %d", fps);
		LOG("PARTICLES", "Num Particles: %d", particles);
#endif
		mFrameTime -= 0.5f;
		mFrameCount = 0;
	}

	mTestTime += deltaTime;
	if (mTestTime >= mTestStartTime
		&& mTestTime <= mTestEndTime)
	{
		mRealTestTime += deltaTime;
		mTestFrameCount++;
	}
	if (mTestTime > mTestEndTime
		&& !mTestFinished)
	{
		float avgFps = mTestFrameCount / mRealTestTime;
		float avgFrameTime = 1.0f / avgFps;
		std::string mode =
#if CPU
			"CPU";
#if PARALLEL
			mode += " (PARALLEL)";
#endif
#if INSTANCE
			mode += " (INSTANCE)";
#endif
#if INDEXED
			mode += " (INDEXED)";
#endif
#endif
#if TF
			"TF";
#endif
#if FS
			"FS";
#endif
#if CS
			"CS (" + std::to_string(WORK_GROUP_SIZE) + ")";
#endif
#if SORT
			mode += " (SORT)";
#endif
		LOG("RESULT:", "\n\tMODE: %s\n\tEmit-Rate (Particles/s): %g\n\tTest Time: %g\n\tAvg. FPS: %g\n\tAvg. Frame-time: %g\n\tParticles: %d", mode.c_str(), mEmitRate, mRealTestTime, avgFps, avgFrameTime, particles);
		//mTestFinished = true;
		mCurrentTestRun++;
		mTestResults.push_back(avgFrameTime);
		if (mCurrentTestRun < mTestRuns)
			ReInit();
		else
		{
			mTestFinished = true;
			std::string result;
			for (auto it = mTestResults.begin(); it != mTestResults.end(); it++)
			{
				result += "\n";
				result += std::to_string(*it);
			}
			LOG("SUMMARY: ", "%s", result.c_str());
		}
	}

	mLastFrameTime = now;
}

bool TestApp::ShouldClose() const
{
	return mTestFinished;
}

void TestApp::ProcessLookInput(float deltaX, float deltaY)
{
	mCamera.ProcessMouseMovement(deltaX, deltaY);
}

void TestApp::Zoom(float zoom)
{
	mCamera.Zoom += zoom;
}