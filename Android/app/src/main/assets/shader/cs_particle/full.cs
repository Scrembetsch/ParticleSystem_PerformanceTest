#version VERSION

precision mediump float;

layout(LOCAL_WORK_GROUP_SIZE) in;

layout(std140, binding=4) buffer Position
{
    vec4 Positions[];
};

layout(std140, binding=5) buffer Velocity
{
    vec4 Velocities[];
};

layout(std140, binding=6) buffer Color
{
    vec4 Colors[];
};

uniform float uDeltaTime;

void main()
{
    const vec3 cGravity = vec3(0.0, 0.0, 0.0);
    // uint gid = gl_GlobalInvocationID.x * 4U;

    // for(uint i = 0U; i < 4U; i++)
    // {
    //     vec3 p = Positions[gid + i].xyz;
    //     vec3 v = Velocities[gid + i].xyz;

    //     vec3 pp = p + v * uDeltaTime + 0.5 * uDeltaTime * uDeltaTime * cGravity;
    //     vec3 vp = v + cGravity * uDeltaTime;

    //     Positions[gid + i].xyz = pp;
    //     Velocities[gid + i].xyz = vp;
    // }

    uint gid = gl_GlobalInvocationID.x;

    vec3 p = Positions[gid].xyz;
    vec3 v = Velocities[gid].xyz;

    vec3 pp = p + v * uDeltaTime + 0.5 * uDeltaTime * uDeltaTime * cGravity;
    vec3 vp = v + cGravity * uDeltaTime;

    Positions[gid].xyz = pp;
    Velocities[gid].xyz = vp;
}