#pragma once

#include "../glm/glm.hpp"

struct TfParticle
{
    enum Type
    {
        GENRATOR = 0,
        PARTICLE = 1
    };

    glm::vec3 Position = glm::vec3(0.0);
    glm::vec3 Velocity = glm::vec3(0.0);
    glm::vec4 Color = glm::vec4(1.0);

    float LifeTime = 0.0f;
    float BeginLifeTime = 0.0f;
    float Type = 0.0f;
};