#pragma once

#include "gl.h"

class Texture
{
public:
    Texture(GLuint tex = 0, GLuint texLocation = GL_TEXTURE0)
        : mTex(tex)
        , mTexLocation(texLocation)
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

    virtual void Use(GLuint overrideLocation = GL_TEXTURE) const = 0;

    unsigned int mTex;
    unsigned int mTexLocation;
    bool mShared;
};
