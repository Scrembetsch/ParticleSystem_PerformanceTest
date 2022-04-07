#pragma once

#include "gl.h"

#include <string>

#if _DEBUG
#define CHECK_GL_ERROR() GlUtil::CheckGlError((std::string(__FILE__) + std::to_string(__LINE__)).c_str())
#else
#define CHECK_GL_ERROR() GlUtil::CheckGlError((std::string(__FILE__) + std::to_string(__LINE__)).c_str())
#endif

namespace GlUtil
{
    bool CheckGlError(const char* functionName);
    void PrintGlString(const char* name, GLenum glEnumType);

    GLuint LoadTexture(const std::string& filepath);
};
