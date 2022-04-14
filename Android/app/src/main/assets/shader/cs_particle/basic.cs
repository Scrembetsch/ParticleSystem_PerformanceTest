#version VERSION

precision mediump float;

layout(LOCAL_WORK_GROUP_SIZE) in;

layout(binding = 0) uniform atomic_uint NumGenerated;

layout(std140, binding=1) buffer Position
{
    vec3 Positions[];
};

layout(std140, binding=2) buffer Velocity
{
    vec3 Velocities[];
};

layout(std140, binding=3) buffer Color
{
    vec4 Colors[];
};

layout(std140, binding=4) buffer Lifetime
{
    vec2 Lifetimes[];
};

uniform float uDeltaTime;
uniform uint uNumToGenerate;

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

void InitParticle()
{
    uint gid = gl_GlobalInvocationID.x;

    Positions[gid].xyz = vec3(0.0);
    Velocities[gid].xyz = vec3(randZeroOne(), randZeroOne(), randZeroOne());
    Colors[gid].rgba = vec4(1.0);
    Lifetimes[gid].xy = vec2(5.0);
}

void main()
{
    uint gid = gl_GlobalInvocationID.x;
    lLocalSeed = vec3(gid, uDeltaTime, uDeltaTime * uDeltaTime);

    Colors[gid].rg = vec2(uNumToGenerate, atomicCounter(NumGenerated));
    Colors[gid].ba = vec2(0.0);

    if(Lifetimes[gid].x <= 0.0)
    {
        if(uNumToGenerate > atomicCounter(NumGenerated)
            && uNumToGenerate > atomicCounterIncrement(NumGenerated))
        {
            InitParticle();
        }
        return;
    }

    const vec3 cGravity = vec3(0.0, 0.0, 0.0);

    vec3 p = Positions[gid].xyz;
    vec3 v = Velocities[gid].xyz;

    vec3 pp = p + v * uDeltaTime + 0.5 * uDeltaTime * uDeltaTime * cGravity;
    vec3 vp = v + cGravity * uDeltaTime;

    Positions[gid].xyz = pp;
    Velocities[gid].xyz = vp;

    Lifetimes[gid].x -= uDeltaTime;
}