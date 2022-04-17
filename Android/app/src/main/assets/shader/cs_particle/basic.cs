#version VERSION

precision mediump float;

layout(local_size_x = LOCAL_SIZE_X) in;

layout(binding = 0) uniform atomic_uint NumToGenerate;
layout(binding = 0) uniform atomic_uint NumAlive;

// layout(binding = 0) uniform atomic_uint NumToGenerate[DISPATCH_SIZE];
// layout(binding = 0, offset = ATOMIC_OFFSET1) uniform atomic_uint NumAlive[DISPATCH_SIZE];

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

vec3 lLocalSeed;

float randZeroOne()
{
    uint n = floatBitsToUint(lLocalSeed.y * 214013.0 + lLocalSeed.x * 2531011.0 + lLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    lLocalSeed = vec3(lLocalSeed.x + 147158.0 * fRes, lLocalSeed.y * fRes  + 415161.0 * fRes, lLocalSeed.z + 324154.0 * fRes);
    return fRes;
}

void InitParticle(uint id)
{
    Positions[id].xyz = vec3(0.0);
    Velocities[id].xyz = vec3(randZeroOne(), randZeroOne(), randZeroOne());
    Colors[id].rgba = vec4(1.0);
    Lifetimes[id].xy = vec2(5.0);
}

void main()
{
    uint gid = gl_GlobalInvocationID.x;

    lLocalSeed = vec3(gid, uDeltaTime, uDeltaTime * uDeltaTime);

    if(Lifetimes[gid].x <= 0.0)
    {
        if(atomicCounterDecrement(NumToGenerate) < (-1U / 2U))
        {
            InitParticle(gid);
        }
        else
        {
            return;
        }
    }
    uint index = atomicCounterIncrement(NumAlive);
    INDEX_BUFFER_SET_ID
    const vec3 cGravity = vec3(0.0, 0.0, 0.0);

    vec3 p = Positions[gid].xyz;
    vec3 v = Velocities[gid].xyz;

    vec3 pp = p + v * uDeltaTime + 0.5 * uDeltaTime * uDeltaTime * cGravity;
    vec3 vp = v + cGravity * uDeltaTime;

    float lt = Lifetimes[gid].x;

    lt -= uDeltaTime;

    Positions[gid].xyz = pp;
    Velocities[gid].xyz = vp;

    Lifetimes[gid].x = lt;

    // Colors[gid].r = atomicCounter(MaxParticles) / 1000000;
    // Colors[gid].g = atomicCounter(NumGenerated) / 100000;
    // // Colors[gid].gb = vec2(0.0f);
    // Colors[gid].b = 0.0f;
    // Colors[gid].a = 1.0f;
    Colors[gid] = vec4(1.0);
}