#pragma once

#include "texture.h"

class Texture2D : public Texture
{
public:
    Texture2D(GLuint tex = 0, GLuint texLocation = GL_TEXTURE0)
        :Texture(tex, texLocation)
    {
    }

    ~Texture2D() override = default;

    void Use(GLuint overrideLocation = GL_TEXTURE) const override
    {
        if (overrideLocation == GL_TEXTURE)
        {
            glActiveTexture(mTexLocation);
        }
        else
        {
            glActiveTexture(overrideLocation);
        }
        glBindTexture(GL_TEXTURE_2D, mTex);
    }
};
