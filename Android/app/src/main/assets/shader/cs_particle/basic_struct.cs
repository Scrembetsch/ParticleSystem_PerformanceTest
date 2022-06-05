#version VERSION

precision mediump float;

layout(local_size_x = LOCAL_SIZE_X) in;

MODULE_ATOMIC_COUNTERS

struct Particle
{
    vec4 Position;
    vec4 Velocity;
    vec4 Color;
    vec4 Lifetime;
};

// Buffer always uses vec4
layout(std140, binding=1) buffer ParticleBuffer
{
    Particle Particles[];
};

INDEX_BUFFER_DECL

uniform float uDeltaTime;
uniform float uLifeTimeMin;
uniform float uLifeTimeRange;
uniform vec3 uPosition;
uniform vec3 uRandomSeed;
uniform vec3 uCameraPos;
uniform vec3 uVelocityMin;
uniform vec3 uVelocityRange;

MODULE_UNIFORMS

vec3 lLocalSeed;
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

void InitParticle()
{
    lParticle.Position.xyz = uPosition;
    lParticle.Velocity.xyz = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
    lParticle.Color.rgba = vec4(1.0);
    lParticle.Lifetime.x = uLifeTimeMin + uLifeTimeRange * randZeroOne();
    lParticle.Lifetime.y = lParticle.Lifetime.x;
}

MODULE_METHODS

void main()
{
    uint gid = gl_GlobalInvocationID.x;
    GenSeed();

    lParticle = Particles[gid];

    MODULE_CALLS

    if(lParticle.Lifetime.x <= 0.0)
    {
        Particles[gid].Position.w = -1.0;
        return;
    }

    uint index = atomicCounterIncrement(NumAlive);
    INDEX_BUFFER_SET_ID

    lParticle.Position = lParticle.Position + lParticle.Velocity * uDeltaTime;
    lParticle.Lifetime.x -= uDeltaTime;

    lParticle.Position.xyzw = vec4(lParticle.Position.xyz, distance(lParticle.Position.xyz, uCameraPos));

    Particles[gid] = lParticle;
}