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

struct CpuRenderParticle
{
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoord;

    static const uint32_t PositionSize = sizeof(Position) / sizeof(float);
    static const uint32_t PositionRealSize = sizeof(Position);
    static const uint32_t ColorSize = sizeof(Color) / sizeof(float);
    static const uint32_t ColorRealSize = sizeof(Color);
    static const uint32_t TexCoordSize = sizeof(TexCoord) / sizeof(float);
    static const uint32_t TexCoordRealSize = sizeof(TexCoord);

    static const uint32_t ParticleSize = PositionSize + ColorSize + TexCoordSize;
    static const uint32_t ParticleRealSize = PositionRealSize + ColorRealSize + TexCoordRealSize;
};