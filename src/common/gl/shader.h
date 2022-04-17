#pragma once

#include "gl.h"
#include "../glm/glm.hpp"
#include "../device/file_handler.h"
#include "../logger.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

class Shader
{
public:
    enum ShaderType
    {
        SHADER_TYPE_VERTEX,
        SHADER_TYPE_FRAGMENT,
        SHADER_TYPE_GEOMETRY,
        SHADER_TYPE_TESS_CONTROL,
        SHADER_TYPE_TESS_EVALUATION,
        SHADER_TYPE_COMPUTE,

        NUM_SHADER_TYPES
    };

private:
    GLuint mId;
    GLuint mShaderLocations[NUM_SHADER_TYPES];
    bool mAttached;
    bool mLinked;

public:
    Shader()
        : mId(0)
        , mAttached(false)
        , mLinked(false)
    {
        for(uint32_t i = 0; i < NUM_SHADER_TYPES; i++)
        {
            mShaderLocations[i] = 0;
        }
    }

    ~Shader()
    {
        if(mId != 0)
        {
            glDeleteProgram(mId);
            mId = 0;
        }
        for(uint32_t i = 0; i < NUM_SHADER_TYPES; i++)
        {
            if(mShaderLocations[i] != 0)
            {
                glDeleteShader(mShaderLocations[i]);
                mShaderLocations[i] = 0;
            }
        }
    }

    GLuint GetId() const
    {
        return mId;
    }

    bool LoadAndCompile(const char* path, ShaderType shaderType, const std::vector<std::pair<std::string, std::string>>& replaceParts = std::vector<std::pair<std::string, std::string>>())
    {
        if(mLinked)
        {
            LOGE("SHADER", "Shaders already linked! Loading not possible!");
        }
        if(mAttached)
        {
            LOGE("SHADER", "Shaders already attached! Loading not possible!");
            return false;
        }
        else
        {
            if(mShaderLocations[shaderType] != 0)
            {
                LOGE("SHADER", "This shader type was already loaded!");
                return false;
            }
        }

        std::string shaderCode;
        FileHandler::Instance().get()->ReadFile(path, shaderCode);
        std::string shaderTypeName = GetShaderTypeName(shaderType);
        uint32_t glShaderType = GetShaderGlType(shaderType);
        if(shaderCode.empty())
        {
            LOGE("SHADER", "Shader file empty or not found!\n ShaderType: %s\n Path: %s", shaderTypeName.c_str(), path);
            return false;
        }

        FindAndIncludeFiles(shaderCode);
        FindAndReplaceStrings(shaderCode, replaceParts);
#if _WIN32
        ReplaceStringPart(shaderCode, "VERSION", "460");
#else
        ReplaceStringPart(shaderCode, "VERSION", "320 es");
#endif

        const char* shaderCodeAsCStr = shaderCode.c_str();
        mShaderLocations[shaderType] = glCreateShader(glShaderType);
        glShaderSource(mShaderLocations[shaderType], 1, &shaderCodeAsCStr, nullptr);
        glCompileShader(mShaderLocations[shaderType]);

        if(CheckShaderCompileError(mShaderLocations[shaderType], shaderTypeName, path))
        {
            return false;
        }

        return true;
    }

    bool AttachLoadedShaders()
    {
        if(mAttached)
        {
            LOGE("SHADER", "Shaders are already attached!");
            return false;
        }
        if(mLinked)
        {
            LOGE("SHADER", "Shaders are already linked!");
            return false;
        }

        mId = glCreateProgram();
        uint32_t attachedShaders = 0;
        for(uint32_t i = 0; i < NUM_SHADER_TYPES; i++)
        {
            if(mShaderLocations[i] != 0)
            {
                glAttachShader(mId, mShaderLocations[i]);
                glDeleteShader(mShaderLocations[i]);
                mShaderLocations[i] = 0;
                attachedShaders++;
            }
        }
        if(CheckProgramAttachError(mId, attachedShaders))
        {
            return false;
        }
        mAttached = true;
        return true;
    }

    bool Link()
    {
        if(!mAttached)
        {
            LOGE("SHADER", "Shaders need to be attached before they are linked!");
            return false;
        }
        if(mLinked)
        {
            LOGE("SHADER", "Shaders are already linked!");
            return false;
        }

        glLinkProgram(mId);
        if(CheckProgramLinkError(mId))
        {
            return false;
        }
        mLinked = true;
        return true;
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void Use() const
    {
        glUseProgram(mId);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetBool(const std::string &name, bool value) const
    {
        glUniform1i(GetLocation(name), (int)value);
    }
    // ------------------------------------------------------------------------
    void SetInt(const std::string &name, int value) const
    {
        glUniform1i(GetLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void SetUInt(const std::string& name, unsigned int value) const
    {
        glUniform1ui(GetLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void SetFloat(const std::string &name, float value) const
    {
        glUniform1f(GetLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void SetVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(GetLocation(name), 1, &value[0]);
    }
    void SetVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(GetLocation(name), x, y);
    }
    // ------------------------------------------------------------------------
    void SetUVec3(const std::string& name, const glm::uvec3& value) const
    {
        glUniform3uiv(GetLocation(name), 1, &value[0]);
    }
    void SetUVec3(const std::string& name, uint32_t x, uint32_t y, uint32_t z) const
    {
        glUniform3ui(GetLocation(name), x, y, z);
    }
    // ------------------------------------------------------------------------
    void SetVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(GetLocation(name), 1, &value[0]);
    }
    void SetVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(GetLocation(name), x, y, z);
    }
    // ------------------------------------------------------------------------
    void SetVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(GetLocation(name), 1, &value[0]);
    }
    void SetVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(GetLocation(name), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void SetMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void SetMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void SetMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

private:
    std::string GetShaderTypeName(ShaderType shaderType) const
    {
        switch(shaderType)
        {
            case SHADER_TYPE_VERTEX:
                return "VERTEX";
            case SHADER_TYPE_FRAGMENT:
                return "FRAGMENT";
            case SHADER_TYPE_GEOMETRY:
                return "GEOMETRY";
            case SHADER_TYPE_TESS_CONTROL:
                return "TESS_CONTROL";
            case SHADER_TYPE_TESS_EVALUATION:
                return "TESS_EVALUATION";
            case SHADER_TYPE_COMPUTE:
                return "COMPUTE";
            default:
                return "TYPE_NOT_DEFINED";
        }
    }

    uint32_t GetShaderGlType(ShaderType shaderType)
    {
        switch(shaderType)
        {
            case SHADER_TYPE_VERTEX:
                return GL_VERTEX_SHADER;
            case SHADER_TYPE_FRAGMENT:
                return GL_FRAGMENT_SHADER;
            case SHADER_TYPE_GEOMETRY:
                return GL_GEOMETRY_SHADER;
            case SHADER_TYPE_TESS_CONTROL:
                return GL_TESS_CONTROL_SHADER;
            case SHADER_TYPE_TESS_EVALUATION:
                return GL_TESS_EVALUATION_SHADER;
            case SHADER_TYPE_COMPUTE:
                return GL_COMPUTE_SHADER;
            default:
                return GL_INVALID_ENUM;
        }
    }
    bool CheckShaderCompileError(GLuint shader, const std::string& type, const std::string& path)
    {
        GLint isCompiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (!isCompiled)
        {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

            LOGE("SHADER", "ERROR::SHADER_COMPILATION_ERROR of type: %s\nPath:%s\n%s\n -------------------------------------- \n", type.c_str(), path.c_str(), &errorLog[0]);
            return true;
        }
        return false;
    }

    bool CheckProgramAttachError(GLuint program, uint32_t numShaders)
    {
        GLint attachedShaders;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &attachedShaders);
        if (attachedShaders != numShaders)
        {
            LOGE("SHADER", "ERROR::PROGRAM_ATTACH_FAILED: Attached Shaders: %d, Should be: %d\n -------------------------------------- \n", attachedShaders, numShaders);

            glValidateProgram(program);
            glGetProgramiv(program, GL_VALIDATE_STATUS, &attachedShaders);

            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> errorLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

            LOGE("SHADER", "ERROR::PROGRAM_ATTACH_FAILED : %s\n -------------------------------------- \n", &errorLog[0]);
            return true;
        }
        return false;
    }

    bool CheckProgramLinkError(GLuint program)
    {
        GLint success;
        glValidateProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> errorLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

            LOGE("SHADER", "ERROR::PROGRAM_LINK_FAILED : %s\n -------------------------------------- \n", &errorLog[0]);
            return true;
        }
        return false;
    }

    void FindAndIncludeFiles(std::string& code) const
    {
        size_t pos = 0;
        while ((pos = code.find("INCLUDE")) != std::string::npos)
        {
            size_t fileBegin = code.find("\"", pos);
            size_t fileEnd = code.find("\"", fileBegin + 1);

            std::string fileName = code.substr(fileBegin + 1, fileEnd - fileBegin - 1);

            std::string includeCode;
            FileHandler::Instance()->ReadFile(fileName, includeCode);

            code.replace(code.begin() + pos, code.begin() + fileEnd + 1, includeCode);
        }
    }

    void ReplaceStringPart(std::string& original, const std::string& searchText, const std::string& replaceText)
    {
        size_t pos = 0u;
        while ((pos = original.find(searchText, pos)) != std::string::npos)
        {
            original.replace(pos, searchText.length(), replaceText);
            pos += replaceText.length();
        }
    }

    void FindAndReplaceStrings(std::string& code, const std::vector<std::pair<std::string, std::string>>& replaceParts)
    {
        uint32_t size = replaceParts.size();
        for(uint32_t i = 0; i < size; i++)
        {
            ReplaceStringPart(code, replaceParts[i].first, replaceParts[i].second);
        }
    }

    GLint GetLocation(const std::string& name) const
    {
        GLint location = glGetUniformLocation(mId, name.c_str());
        //assert(location != -1);
        return location;
    }
};
