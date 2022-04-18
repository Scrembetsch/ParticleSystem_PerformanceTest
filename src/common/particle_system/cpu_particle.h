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
        //return this->Lifetime > other.Lifetime;
    }
};

struct CpuRenderParticle
{
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoord;

    static const size_t PositionSize = sizeof(Position) / sizeof(float);
    static const size_t PositionRealSize = sizeof(Position);
    static const size_t ColorSize = sizeof(Color) / sizeof(float);
    static const size_t ColorRealSize = sizeof(Color);
    static const size_t TexCoordSize = sizeof(TexCoord) / sizeof(float);
    static const size_t TexCoordRealSize = sizeof(TexCoord);

    static const size_t ParticleSize = PositionSize + ColorSize + TexCoordSize;
    static const size_t ParticleRealSize = PositionRealSize + ColorRealSize + TexCoordRealSize;
};

struct CpuInstanceRenderParticle
{
    glm::vec3 Position;
    glm::vec4 Color;

    static const size_t PositionSize = sizeof(Position) / sizeof(float);
    static const size_t PositionRealSize = sizeof(Position);
    static const size_t ColorSize = sizeof(Color) / sizeof(float);
    static const size_t ColorRealSize = sizeof(Color);

    static const size_t ParticleSize = PositionSize + ColorSize;
    static const size_t ParticleRealSize = PositionRealSize + ColorRealSize;
};