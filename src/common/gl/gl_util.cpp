#include "gl_util.h"

#include "../../common/logger.h"
#include "../device/file_handler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../util/stb_image.h"

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
            errorName = "GL_INVALID_ENUM";
            break;

        case GL_INVALID_VALUE:
            errorName = "GL_INVALID_VALUE";
            break;

        case GL_INVALID_OPERATION:
            errorName = "GL_INVALID_OPERATION";
            break;

        default:
            break;
        }
        LOGE("GL_UTIL", "GL error after %s(): 0x%08x, %s\n", functionName, error, errorName);
        return true;
    }
    while ((error = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        const char* errorName = "Error not found!";
        switch (error)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                errorName = "GL_FRAMEBUFFER_UNDEFINED";
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                errorName = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                errorName = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;

            case GL_FRAMEBUFFER_UNSUPPORTED:
                errorName = "GL_FRAMEBUFFER_UNSUPPORTED";
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                errorName = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                errorName = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
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

GLuint GlUtil::LoadTexture(const std::string& filepath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    char* fileData = nullptr;
    size_t size = 0;
    FileHandler::Instance()->ReadFile(filepath, &fileData, size);

    unsigned char* data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(fileData), size, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        LOGE("GL_UTIL", "Texture failed to load at path: %s", filepath.c_str());
        stbi_image_free(data);
    }
    return textureID;
}