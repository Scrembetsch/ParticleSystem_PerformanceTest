#version VERSION

precision mediump float;

layout(local_size_x = LOCAL_SIZE_X) in;

MODULE_ATOMIC_COUNTERS

// Buffer always uses vec4
layout(std140, binding=1) buffer Position
{
    vec4 Positions[];
};

layout(std140, binding=2) buffer Velocity
{
    vec4 Velocities[];
};

layout(std140, binding=3) buffer Color
{
    vec4 Colors[];
};

layout(std140, binding=4) buffer Lifetime
{
    vec4 Lifetimes[];
};

INDEX_BUFFER_DECL

uniform float uDeltaTime;
uniform vec3 uPosition;
uniform vec3 uRandomSeed;
uniform vec3 uCameraPos;
uniform vec3 uVelocityMin;
uniform vec3 uVelocityRange;
uniform float uLifeTimeMin;
uniform float uLifeTimeRange;

MODULE_UNIFORMS

vec3 lLocalSeed;

struct Particle
{
    vec3 Position;
    float DistanceToCamera;
    vec3 Velocity;
    vec4 Color;
    float CurrentLifetime;
    float BeginLifetime;
    float LifetimeT;
    bool Alive;
    float AliveF;
};
Particle lParticle;

float randZeroOne()
{
    uint n = floatBitsToUint(lLocalSeed.y * 2.113 + lLocalSeed.x * 1.892 + lLocalSeed.z * 1.4234);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    lLocalSeed = vec3(lLocalSeed.x + 14158.0 * fRes, lLocalSeed.y * fRes  + 411.0 * fRes, lLocalSeed.z + 254.0 * fRes);
    return fRes;
}

void GenSeed()
{
    lLocalSeed = vec3(uRandomSeed.x + float(gl_GlobalInvocationID.x) / 20.0, uRandomSeed.y - float(gl_GlobalInvocationID.x) / 20.0, uRandomSeed.z - float(gl_GlobalInvocationID.x) / 50.0);
}

void InitLocalParticle()
{
    uint gid = gl_GlobalInvocationID.x;

    lParticle.Position = Positions[gid].xyz;
    lParticle.Velocity = Velocities[gid].xyz;
    lParticle.Color = Colors[gid].rgba;
    lParticle.CurrentLifetime = Lifetimes[gid].x - uDeltaTime;
    lParticle.BeginLifetime = Lifetimes[gid].y;
    lParticle.LifetimeT = lParticle.CurrentLifetime / lParticle.BeginLifetime;
    lParticle.Alive = lParticle.CurrentLifetime > 0.0;
    lParticle.AliveF = float(lParticle.Alive);
}

void WriteParticleToStorage()
{
    uint gid = gl_GlobalInvocationID.x;

    Positions[gid] = vec4(lParticle.Position, lParticle.DistanceToCamera);
    Velocities[gid].xyz = lParticle.Velocity;
    Colors[gid] = lParticle.Color;
    Lifetimes[gid].xy = vec2(lParticle.CurrentLifetime, lParticle.BeginLifetime);
}

void InitParticle()
{
    lParticle.Position = uPosition;
    lParticle.Velocity = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
    lParticle.Color = vec4(1.0);
    lParticle.BeginLifetime = uLifeTimeMin + uLifeTimeRange * randZeroOne();
    lParticle.CurrentLifetime = lParticle.BeginLifetime;
    lParticle.LifetimeT = 1.0;
    lParticle.Alive = true;
}

MODULE_METHODS

void main()
{
    uint gid = gl_GlobalInvocationID.x;
    GenSeed();

    InitLocalParticle();

    MODULE_CALLS

    if(!lParticle.Alive)
    {
        lParticle.DistanceToCamera = -1.0;
        WriteParticleToStorage();
        return;
    }

    uint index = atomicCounterIncrement(NumAlive);
    INDEX_BUFFER_SET_ID

    lParticle.Position = lParticle.Position + lParticle.Velocity * uDeltaTime;
    lParticle.DistanceToCamera = distance(lParticle.Position, uCameraPos);

    WriteParticleToStorage();
}