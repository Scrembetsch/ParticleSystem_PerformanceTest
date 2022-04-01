#include "test_app.h"

#include "gl/gl.h"
#include "gl/gl_util.h"

TestApp::TestApp()
	: mWidth(0.0f)
	, mHeight(0.0f)
	, mFrameCount(0)
	, mFrameTime(0.0f)
{
}

TestApp::~TestApp()
{
}

void TestApp::Resize(uint32_t width, uint32_t height)
{
	mWidth = width;
	mHeight = height;
	glViewport(0, 0, width, height);
}

bool TestApp::Init()
{
	bool success = true;
#if CPU
	success &= mCpuParticleSystem.Init();
	success &= mCpuShader.LoadAndCompile("shader/base.vs", Shader::ShaderType::SHADER_TYPE_VERTEX);
	success &= mCpuShader.LoadAndCompile("shader/base.fs", Shader::ShaderType::SHADER_TYPE_FRAGMENT);
	success &= mCpuShader.AttachLoadedShaders();
	success &= mCpuShader.Link();
#endif
#if CS
	success &= mCsParticleSystem.Init();
#endif
#if TF
	success &= mTfParticleSystem.Init();
#endif

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mCamera.Position = glm::vec3(-2.57005f, 16.0659f, -31.8907f);
	mCamera.Yaw = 90.0f;
	mCamera.Pitch = -18.0f;
	mCamera.UpdateCameraVectors();

	mLastFrameTime = std::chrono::system_clock::now();
	return success;
}

void TestApp::Step()
{
	auto now = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsedSeconds = now - mLastFrameTime;
	float deltaTime = elapsedSeconds.count();

	glm::mat4 projection = glm::perspective(glm::radians(mCamera.Zoom), mWidth / mHeight, 0.1f, 200.0f);
	glm::mat4 view = mCamera.GetViewMatrix();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if CPU
	mCpuParticleSystem.UpdateParticles(deltaTime);
	mCpuShader.Use();
	mCpuShader.SetMat4("uProjection", projection);
	mCpuShader.SetMat4("uView", view);
	mCpuParticleSystem.RenderParticles();
#endif
#if CS
	mCsParticleSystem.Update(deltaTime);
	mCsParticleSystem.PrepareRender(projection, view, mCamera.Up, mCamera.Front);
	mCsParticleSystem.Render();
#endif
#if TF
	mTfParticleSystem.UpdateParticles(deltaTime);
	mTfParticleSystem.SetMatrices(projection, view, mCamera.Front, mCamera.Up);
	mTfParticleSystem.RenderParticles();
#endif

	GlUtil::CheckGlError("Step");

	mFrameTime += deltaTime;
	mFrameCount++;
	if (mFrameTime > 0.5f)
	{
		int fps = static_cast<int>(round(static_cast<float>(mFrameCount) / mFrameTime));
		LOG("FRAME_TIME", "Fps: %d", fps);

		mFrameTime -= 0.5f;
		mFrameCount = 0;
	}

	mLastFrameTime = now;
}

void TestApp::ProcessLookInput(float deltaX, float deltaY)
{
	mCamera.ProcessMouseMovement(deltaX, deltaY);
}