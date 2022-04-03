#pragma once

#include "gl.h"
#include "shader.h"

#include <string>

class Texture
{
public:
    Texture(GLuint tex = 0, GLuint texLocation = GL_TEXTURE0, const std::string& name = std::string())
        : mTex(tex)
        , mTexLocation(texLocation)
        , mTexName(name)
        , mShared(false)
    {
    }

    virtual ~Texture()
    {
        if (mTex != 0)
        {
            glDeleteTextures(1, &mTex);
            mTex = 0;
        }
    }

    virtual void Use(Shader* shader = nullptr, GLuint overrideLocation = GL_TEXTURE) const = 0;

    unsigned int mTex;
    unsigned int mTexLocation;
    std::string mTexName;
    bool mShared;
};
