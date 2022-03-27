#pragma once

#include "gl.h"

#include <string>

namespace GlUtil
{
    bool CheckGlError(const char* functionName);
    void PrintGlString(const char* name, GLenum glEnumType);
};
