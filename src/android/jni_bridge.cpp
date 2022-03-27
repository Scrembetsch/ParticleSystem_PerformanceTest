#include "../common/gl.h"
#include "../common/logger.h"
#include "../common/test_app.h"

#include <cstring>
#include <jni.h>

TestApp* g_TestApp;

extern "C" JNIEXPORT jboolean JNICALL Java_at_tributsch_particle_1test_1opengl_1es_JniBridge_Init(JNIEnv* env, jclass obj, jobject assetManager)
{
    if (g_TestApp != nullptr)
    {
        delete g_TestApp;
        g_TestApp = nullptr;
    }

//    g_AssetManager = AAssetManager_fromJava(env, assetManager);
//    FileHandler::Init(g_AssetManager);

    const char* versionStr = (const char*)glGetString(GL_VERSION);
    if (strstr(versionStr, "OpenGL ES 3."))
    {
        g_TestApp = new TestApp();
        if(g_TestApp == nullptr)
        {
            return false;
        }
    }
    else
    {
        LOGE("JNI_BRIDGE", "Unsupported OpenGL ES version");
        return false;
    }
    return true;
}

extern "C" JNIEXPORT void JNICALL Java_at_tributsch_particle_1test_1opengl_1es_JniBridge_Resize(JNIEnv* env, jclass obj, jint width, jint height)
{
    if (g_TestApp)
    {
        g_TestApp->Resize(width, height);
    }
}

extern "C" JNIEXPORT void JNICALL Java_at_tributsch_particle_1test_1opengl_1es_JniBridge_Step(JNIEnv* env, jclass obj)
{
    if (g_TestApp)
    {
        g_TestApp->Step();
    }
}