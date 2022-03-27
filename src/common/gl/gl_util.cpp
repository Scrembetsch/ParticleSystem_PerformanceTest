#include "gl_util.h"
#include "../../common/logger.h"

#include <malloc.h>

bool GlUtil::CheckGlError(const char *functionName)
{
    GLint error = glGetError();
    if(error != GL_NO_ERROR)
    {
        LOGE("GL_UTIL", "GL error after %s(): 0x%08x\n", functionName, error);
        PrintGlString("ERROR", error);
        return true;
    }
    return false;
}

void GlUtil::PrintGlString(const char *name, GLenum glEnumType)
{
    const char* glString = (const char*)glGetString(glEnumType);
    LOGD("GL_UTIL", "GL %s: %s\n", name, glString);
}
