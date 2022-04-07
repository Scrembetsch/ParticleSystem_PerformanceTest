#pragma once

#include "gl.h"

#include <string>

#if _DEBUG
#define CHECK_GL_ERROR(funcName) GlUtil::CheckGlError(funcName)
#else
#define CHECK_GL_ERROR(funcName)
#endif

namespace GlUtil
{
    bool CheckGlError(const char* functionName);
    void PrintGlString(const char* name, GLenum glEnumType);

    GLuint LoadTexture(const std::string& filepath);
};
