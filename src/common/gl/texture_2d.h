#pragma once

#include "texture.h"
#include "gl_util.h"

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
            CHECK_GL_ERROR();

            if (shader != nullptr)
            {
                shader->SetInt(mTexName, location - GL_TEXTURE0);
                CHECK_GL_ERROR();

            }
            glBindTexture(GL_TEXTURE_2D, mTex);
            CHECK_GL_ERROR();

        }
        CHECK_GL_ERROR();
    }
};
