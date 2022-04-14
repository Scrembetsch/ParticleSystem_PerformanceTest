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

#include "particle_system/tf_module_emission.h"
#include "particle_system/tf_module_velocity_over_lifetime.h"
#include "particle_system/tf_module_color_over_lifetime.h"

#ifdef _DEBUG
	uint32_t numParticles = 2;
	float numGenerate = 1;
#else
	uint32_t numParticles = 1024 * 1024;
	float numGenerate = 100000;
#endif

TestApp::TestApp()
	: mFrameCount(0)
	, mFrameTime(0.0f)
{
}

TestApp::~TestApp()
{
}

void TestApp::Resize(uint32_t width, uint32_t height)
{
	mCamera.ViewWidth = width;
	mCamera.ViewHeight = height;
	glViewport(0, 0, width, height);
}

bool TestApp::Init()
{
	mParticleTex.mTex = GlUtil::LoadTexture("textures/particle.png");
	mParticleTex.mTexLocation = GL_TEXTURE0;
	mParticleTex.mTexName = "uDiffuseMap";

	std::vector<std::pair<std::string, std::string>> replaceMap;
#ifdef _WIN32
	replaceMap.emplace_back("DECL_TEX0", "layout (binding = 0) uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#else
	replaceMap.emplace_back("DECL_TEX0", "uniform sampler2D uDiffuseMap;");
	replaceMap.emplace_back("USE_TEX0", "uDiffuseMap");
#endif

	bool success = true;
#if CPU
	mCpuParticleSystem = new
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
		(numParticles
	#if PARALLEL
			, std::thread::hardware_concurrency()
	#endif
			);
	success &= mCpuParticleSystem->Init();

	mCpuParticleSystem->SetMinLifetime(5.0f);
	mCpuParticleSystem->SetMaxLifetime(7.0f);
	mCpuParticleSystem->SetMinStartVelocity(glm::vec3(-2.0f, -2.0f, 0.0f));
	mCpuParticleSystem->SetMaxStartVelocity(glm::vec3(2.0f, 2.0f, 0.0f));
	mCpuParticleSystem->AddModule(new CpuModuleEmission(mCpuParticleSystem, numGenerate));
	mCpuParticleSystem->AddModule(new CpuModuleVelOverLife(mCpuParticleSystem, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	mCpuParticleSystem->AddModule(new CpuModuleColorOverLife(mCpuParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));

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
#if TF
	mTfParticleSystem = new TfParticleSystem(numParticles);

	mTfParticleSystem->SetMinLifetime(5.0f);
	mTfParticleSystem->SetMaxLifetime(7.0f);
	mTfParticleSystem->SetMinStartVelocity(glm::vec3(-2.0f, -2.0f, 0.0f));
	mTfParticleSystem->SetMaxStartVelocity(glm::vec3(2.0f, 2.0f, 0.0f));
	mTfParticleSystem->SetRenderFragReplaceMap(replaceMap);
	mTfParticleSystem->AddModule(new TfModuleEmission(mTfParticleSystem, numGenerate));
	mTfParticleSystem->AddModule(new TfModuleVelOverLife(mTfParticleSystem, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	mTfParticleSystem->AddModule(new TfModuleColorOverLife(mTfParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	success &= mTfParticleSystem->Init();
#endif
#if CS
	mCsParticleSystem = new CsParticleSystem(numParticles, 1);

	mCsParticleSystem->SetMinLifetime(5.0f);
	mCsParticleSystem->SetMaxLifetime(7.0f);
	mCsParticleSystem->SetMinStartVelocity(glm::vec3(-2.0f, -2.0f, 0.0f));
	mCsParticleSystem->SetMaxStartVelocity(glm::vec3(2.0f, 2.0f, 0.0f));
	mCsParticleSystem->SetRenderFragReplaceMap(replaceMap);
	mCsParticleSystem->EmitRate = numGenerate;
	//mCsParticleSystem->AddModule(new TfModuleEmission(mTfParticleSystem, numGenerate));
	//mCsParticleSystem->AddModule(new TfModuleVelOverLife(mTfParticleSystem, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	//mCsParticleSystem->AddModule(new TfModuleColorOverLife(mTfParticleSystem, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	success &= mCsParticleSystem->Init();
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
	auto now = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsedSeconds = now - mLastFrameTime;
	float deltaTime = elapsedSeconds.count();

	glm::mat4 projection = glm::perspective(glm::radians(mCamera.Zoom), mCamera.ViewWidth / mCamera.ViewHeight, 0.1f, 200.0f);
	glm::mat4 view = mCamera.GetViewMatrix();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uint32_t particles = 0;

#if CPU
	mCpuParticleSystem->UpdateParticles(deltaTime, mCamera.Position);
	mCpuShader.Use();
	mCpuShader.SetMat4("uProjection", projection);
	mCpuShader.SetMat4("uView", view);
	mParticleTex.Use(&mCpuShader);
	mCpuParticleSystem->RenderParticles();
	particles = mCpuParticleSystem->GetCurrentParticles();
	CHECK_GL_ERROR();
#endif
#if CS
	mCsParticleSystem->UpdateParticles(deltaTime, mCamera.Position);
	mCsParticleSystem->GetRenderShader()->Use();
	mParticleTex.Use(mCsParticleSystem->GetRenderShader());
	mCsParticleSystem->PrepareRender(&mCamera);
	mCsParticleSystem->RenderParticles();
	particles = mCsParticleSystem->GetCurrentParticles();
	CHECK_GL_ERROR();
#endif
#if TF
	mTfParticleSystem->UpdateParticles(deltaTime, mCamera.Position);
	mTfParticleSystem->GetRenderShader()->Use();
	mParticleTex.Use(mTfParticleSystem->GetRenderShader());
	mTfParticleSystem->PrepareRender(&mCamera);
	mTfParticleSystem->RenderParticles();
	particles = mTfParticleSystem->GetCurrentParticles();
	CHECK_GL_ERROR();
#endif

	mFrameTime += deltaTime;
	mFrameCount++;
	if (mFrameTime > 0.5f)
	{
		int fps = static_cast<int>(round(static_cast<float>(mFrameCount) / mFrameTime));
		LOG("FRAME_TIME", "Fps: %d", fps);
		LOG("PARTICLES", "Num Particles: %d", particles);

		mFrameTime -= 0.5f;
		mFrameCount = 0;
	}

	mLastFrameTime = now;
}

void TestApp::ProcessLookInput(float deltaX, float deltaY)
{
	mCamera.ProcessMouseMovement(deltaX, deltaY);
}

void TestApp::Zoom(float zoom)
{
	mCamera.Zoom += zoom;
}