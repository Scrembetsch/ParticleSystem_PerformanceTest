#pragma once

#include "texture.h"

class Texture2D : public Texture
{
public:
    Texture2D(GLuint tex = 0, GLuint texLocation = GL_TEXTURE0, const std::string& name = std::string())
        :Texture(tex, texLocation, name)
    {
    }

    ~Texture2D() override = default;

    void Use(Shader* shader = nullptr, GLuint overrideLocation = GL_TEXTURE) const override
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

        if (mTex != 0)
        {
            GLuint location = mTexLocation;
            if (overrideLocation != GL_TEXTURE)
            {
                location = overrideLocation;
            }
            glActiveTexture(location);
            if (shader != nullptr)
            {
                shader->SetInt(mTexName, location - GL_TEXTURE0);
            }
            glBindTexture(GL_TEXTURE_2D, mTex);
        }
    }
};
