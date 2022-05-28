#include "test_app.h"

#include "gl/gl.h"
#include "gl/gl_util.h"

#include "particle_system/cpu_serial_particle_system.h"
#include "particle_system/cpu_serial_instance_particle_system.h"
#include "particle_system/cpu_parallel_particle_system.h"
#include "particle_system/cpu_parallel_instance_particle_system.h"

#include "particle_system/tf_full_emit_particle_system.h"

#include "particle_system/cpu_module_emission.h"
#include "particle_system/cpu_module_velocity_over_lifetime.h"
#include "particle_system/cpu_module_color_over_lifetime.h"

#include "particle_system/tf_module_emission.h"
#include "particle_system/tf_module_velocity_over_lifetime.h"
#include "particle_system/tf_module_color_over_lifetime.h"

#include "particle_system/cs_module_emission_struct.h"
#include "particle_system/cs_module_velocity_over_lifetime_struct.h"
#include "particle_system/cs_module_color_over_lifetime_struct.h"
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
#if CPU
	delete mParticleSystem;
#endif

	std::vector<std::pair<std::string, std::string>> replaceMap;
#ifdef _WIN32
	replaceMap.emplace_back("DECL_TEX0", "layout (binding = 0) uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#else
	replaceMap.emplace_back("DECL_TEX0", "uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#endif

	//uint32_t testRuns[] = { 10, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000, 1500000, 2000000 };
	uint32_t testRuns[] = { 5000000 };
	mTestRuns = sizeof(testRuns) / sizeof(uint32_t);

	float emitMulti = 5.0f;
	mMaxParticles = testRuns[mCurrentTestRun];
	mEmitRate = mMaxParticles / emitMulti;

	bool success = true;
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
	mParticleSystem->SetMinStartVelocity(glm::vec3(-2.0f, -2.0f, -1.0f));
	mParticleSystem->SetMaxStartVelocity(glm::vec3(2.0f, 2.0f, 0.0f));
	mParticleSystem->AddModule(new CpuModuleEmission(mParticleSystem, mEmitRate));
	mParticleSystem->AddModule(new CpuModuleVelOverLife(mParticleSystem, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	mParticleSystem->AddModule(new CpuModuleColorOverLife(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));

#if TF
	mParticleSystem = new TfParticleSystem(mMaxParticles);

	mParticleSystem->SetMinLifetime(emitMulti);
	mParticleSystem->SetMaxLifetime(emitMulti);
	mParticleSystem->SetMinStartVelocity(glm::vec3(-2.0f, -2.0f, -1.0f));
	mParticleSystem->SetMaxStartVelocity(glm::vec3(2.0f, 2.0f, 0.0f));
	mParticleSystem->SetRenderFragReplaceMap(replaceMap);
	mParticleSystem->AddModule(new TfModuleEmission(mParticleSystem, NUM_TO_GENERATE));
	mParticleSystem->AddModule(new TfModuleVelOverLife(mParticleSystem, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	mParticleSystem->AddModule(new TfModuleColorOverLife(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	success &= mParticleSystem->Init();
#endif
#if CS
#if USE_STRUCT
	mParticleSystem = new CsParticleSystemStruct(mMaxParticles, WORK_GROUP_SIZE);
#else
	mParticleSystem = new CsParticleSystem(mMaxParticles, WORK_GROUP_SIZE);
#endif

	mParticleSystem->SetMinLifetime(emitMulti);
	mParticleSystem->SetMaxLifetime(emitMulti);
	mParticleSystem->SetMinStartVelocity(glm::vec3(-2.0f, -2.0f, -1.0f));
	mParticleSystem->SetMaxStartVelocity(glm::vec3(2.0f, 2.0f, 0.0f));
	mParticleSystem->SetRenderFragReplaceMap(replaceMap);
	mParticleSystem->AddModule(
#if USE_STRUCT
		new CsModuleEmissionStruct
#else
		new CsModuleEmission
#endif
		(mParticleSystem, mEmitRate));
	mCsParticleSystem->AddModule(
#if USE_STRUCT
		new CsModuleVelOverLifeStruct
#else
		new CsModuleVelOverLife
#endif
		(mParticleSystem, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	mCsParticleSystem->AddModule(
#if USE_STRUCT
		new CsModuleColorOverLifeStruct
#else
		new CsModuleColorOverLife
#endif
		(mParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	success &= mParticleSystem->Init();
#endif

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

	mParticleSystem->UpdateParticles(deltaTime, mCamera.Position);

#if CPU
	glm::mat4 projection = glm::perspective(glm::radians(mCamera.Zoom), mCamera.ViewWidth / mCamera.ViewHeight, 0.1f, 200.0f);
	glm::mat4 view = mCamera.GetViewMatrix();

	mCpuShader.Use();
	mCpuShader.SetMat4("uProjection", projection);
	mCpuShader.SetMat4("uView", view);
	mParticleTex.Use(&mCpuShader);
	mParticleSystem->RenderParticles();
	particles = mParticleSystem->GetCurrentParticles();
	CHECK_GL_ERROR();
#endif
#if CS || TF
	mParticleSystem->GetRenderShader()->Use();
	mParticleTex.Use(mParticleSystem->GetRenderShader());
	mParticleSystem->PrepareRender(&mCamera);
	CHECK_GL_ERROR();
#endif
	mParticleSystem->RenderParticles();
	particles = mParticleSystem->GetCurrentParticles();
	CHECK_GL_ERROR();


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
#if CS
			"CS (" + std::to_string(WORK_GROUP_SIZE) + ")";
#if USE_STRUCT
			mode += " (STRUCT)";
#endif
#endif
#if SORT
			mode += " (SORT)";
#endif
		LOG("RESULT:", "\n\tMODE: %s\n\tEmit-Rate (Particles/s): %g\n\tTest Time: %g\n\tAvg. FPS: %g\n\tAvg. Frame-time: %g\n\tParticles: %d", mode.c_str(), mEmitRate, mRealTestTime, avgFps, avgFrameTime, mParticleSystem->GetCurrentParticles());
		//mTestFinished = true;
		mCurrentTestRun++;
		if (mCurrentTestRun < mTestRuns)
			ReInit();
		else
			mTestFinished = true;
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