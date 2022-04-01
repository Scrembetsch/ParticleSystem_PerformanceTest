#include "../common/gl/gl.h"
#include "../common/logger.h"
#include "../common/test_app.h"
#include "../common/device/file_handler.h"

#include <cstring>
#include <jni.h>
#include <android/asset_manager_jni.h>

TestApp* g_TestApp;

extern "C" JNIEXPORT jboolean JNICALL Java_at_tributsch_particlesystem_1performancetest_JniBridge_Init(JNIEnv* env, jclass obj, jobject assetManager)
{
    if (g_TestApp != nullptr)
    {
        delete g_TestApp;
        g_TestApp = nullptr;
    }

    FileHandler::ManagerInit(AAssetManager_fromJava(env, assetManager));

    const char* versionStr = (const char*)glGetString(GL_VERSION);
    if (strstr(versionStr, "OpenGL ES 3."))
    {
        g_TestApp = new TestApp();
        if(g_TestApp == nullptr)
        {
            return false;
        }
        g_TestApp->Init();
    }
    else
    {
        LOGE("JNI_BRIDGE", "Unsupported OpenGL ES version");
        return false;
    }
    return true;
}

extern "C" JNIEXPORT void JNICALL Java_at_tributsch_particlesystem_1performancetest_JniBridge_Resize(JNIEnv* env, jclass obj, jint width, jint height)
{
    if (g_TestApp)
    {
        g_TestApp->Resize(width, height);
    }
}

extern "C" JNIEXPORT void JNICALL Java_at_tributsch_particlesystem_1performancetest_JniBridge_Step(JNIEnv* env, jclass obj)
{
    if (g_TestApp)
    {
        g_TestApp->Step();
    }
}