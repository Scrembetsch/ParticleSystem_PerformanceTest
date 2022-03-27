#pragma once

#include "../gl/gl_util.h"

#include "shader.h"
#include "texture.h"

#include <vector>

class Material
{
public:
    Material()
        : mTextures()
        , mShader(nullptr)
        , mSharedShader(false)
    {
    }

    ~Material()
    {
        for (auto tex : mTextures)
        {
            if (!tex->mShared)
            {
                delete tex;
            }
        }
        mTextures.clear();
        if (!mSharedShader)
        {
            delete mShader;
        }
    }

    void Use() const
    {
        mShader->Use();

        for (auto tex : mTextures)
        {
            tex->Use();
        }
    }

    void UseSharedShader(Shader* shader)
    {
        if (mShader != nullptr && !mSharedShader)
        {
            delete mShader;
        }
        mSharedShader = true;
        mShader = shader;
    }

    void UseShader(Shader* shader)
    {
        mSharedShader = false;
        mShader = shader;
    }

    Shader* GetShader()
    {
        return mShader;
    }

    std::vector<Texture*> mTextures;

private:
    Shader* mShader;
    bool mSharedShader;
};
