#version VERSION

precision mediump float;

layout(local_size_x = LOCAL_SIZE_X) in;

MODULE_ATOMIC_COUNTERS

struct BufferParticle
{
    vec3 Position;
    float Lifetime;
    vec3 Velocity;
    float BeginLifetime;
    vec4 Color;
};

struct IndexStruct
{
    uint Idx;
    float Distance;
};

layout(std430, binding=1) buffer ParticleBuffer
{
    BufferParticle Particles[];
};

layout(std430, binding=2) buffer Index
{
    IndexStruct Indices[];
};

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

    lParticle.Position = Particles[gid].Position.xyz;
    lParticle.Velocity = Particles[gid].Velocity.xyz;
    lParticle.Color = Particles[gid].Color.rgba;
    lParticle.CurrentLifetime = Particles[gid].Lifetime - uDeltaTime;
    lParticle.BeginLifetime = Particles[gid].BeginLifetime;
    lParticle.LifetimeT = clamp(lParticle.CurrentLifetime / lParticle.BeginLifetime, 0.0, 1.0);
    lParticle.Alive = lParticle.CurrentLifetime > 0.0;
    lParticle.AliveF = float(lParticle.Alive);
}

void WriteParticleToStorage()
{
    uint gid = gl_GlobalInvocationID.x;

    Particles[gid].Position.xyz = lParticle.Position;
    Particles[gid].Velocity.xyz = lParticle.Velocity;
    Particles[gid].Color = lParticle.Color;
    Particles[gid].Lifetime = lParticle.CurrentLifetime;
    Particles[gid].BeginLifetime = lParticle.BeginLifetime;
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
        WriteParticleToStorage();
        return;
    }

    uint index = atomicCounterIncrement(NumAlive);

    lParticle.Position = lParticle.Position + lParticle.Velocity * uDeltaTime;

    Indices[index].Idx = gid;
    Indices[index].Distance = distance(lParticle.Position, uCameraPos);

    WriteParticleToStorage();
}