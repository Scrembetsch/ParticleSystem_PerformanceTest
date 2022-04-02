#pragma once

#include "../glm/glm.hpp"

struct Particle
{
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;

    float Lifetime = 0.0f;
    float BeginLifetime = 0.0f;
    bool Active = false;
    uint32_t Seed = 0;

    float CameraDistance = 0.0f;

    bool operator<(const Particle& other) const
    {
        // Sort in reverse order : far particles drawn first.
        return this->CameraDistance > other.CameraDistance;
    }
};