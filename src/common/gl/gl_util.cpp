#include "gl_util.h"
#include "../../common/logger.h"

#include <malloc.h>

bool GlUtil::CheckGlError(const char *functionName)
{
    GLint error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        const char* errorName = "Error not found!";
        switch (error)
        {
        case GL_INVALID_ENUM:
            errorName = "INVALID ENUM";
            break;

        case GL_INVALID_VALUE:
            errorName = "INVALID_VALUE";
            break;

        case GL_INVALID_OPERATION:
            errorName = "INVALID OPERATION";
            break;

        default:
            break;
        }
        LOGE("GL_UTIL", "GL error after %s(): 0x%08x, %s\n", functionName, error, errorName);
        return true;
    }
    return false;
}

void GlUtil::PrintGlString(const char *name, GLenum glEnumType)
{
    const char* glString = (const char*)glGetString(glEnumType);
    LOGD("GL_UTIL", "GL %s: %s\n", name, glString);
}
