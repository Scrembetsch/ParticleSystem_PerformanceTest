#version VERSION

precision mediump float;

struct Particle
{
    vec4 Position;
    vec4 Velocity;
    vec4 Color;
    vec4 Lifetime;
};

layout(binding=1) buffer ParticleBuffer
{
    Particle Particles[];
};

struct IndexStruct
{
    uint Idx;
    float Distance;
};

layout(std430, binding=5) buffer Index
{
    IndexStruct Indices[];
};

uniform mat4 uProjection;
uniform mat4 uView;

out float vLifetimePass;
out vec4 vColorPass;

void main()
{
    uint id = Indices[gl_VertexID].Idx;    

    gl_Position = vec4(Particles[id].Position.xyz, 1.0);
    vLifetimePass = Particles[id].Lifetime.x;
    vColorPass = Particles[id].Color.rgba;
}