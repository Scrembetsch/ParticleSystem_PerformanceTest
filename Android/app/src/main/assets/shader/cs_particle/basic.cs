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

void InitParticle(uint id)
{
    Positions[id].xyz = uPosition;
    Velocities[id].xyz = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
    Colors[id].rgba = vec4(1.0);
    Lifetimes[id].x = uLifeTimeMin + uLifeTimeRange * randZeroOne();
    Lifetimes[id].y = Lifetimes[id].x;
}

MODULE_METHODS

void main()
{
    uint gid = gl_GlobalInvocationID.x;
    GenSeed();

    vec3 position = Positions[gid].xyz;
    vec3 velocity = Velocities[gid].xyz;
    vec2 lifetimes = Lifetimes[gid].xy;
    vec4 color = Colors[gid].rgba;

    MODULE_CALLS

    bool alive = lifetimes.x > 0.0;
    uint aliveInt = alive ? 1U : 0U;
    float aliveFloat = float(aliveInt);

    if(!alive)
    {
        Positions[gid].w = -1.0;
        return;
    }

    uint index = atomicCounterIncrement(NumAlive);
    INDEX_BUFFER_SET_ID

    position = position + velocity * uDeltaTime;
    lifetimes.x -= uDeltaTime;

    Positions[gid].xyzw = vec4(position, distance(position, uCameraPos));
    Velocities[gid].xyz = velocity;
    Lifetimes[gid].x = lifetimes.x;
    Colors[gid] = color;
}