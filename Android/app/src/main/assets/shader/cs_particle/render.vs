#version VERSION

precision mediump float;

layout(shared, binding=1) buffer Position
{
    vec4 Positions[];
};

layout(shared, binding=2) buffer Velocity
{
    vec4 Velocities[];
};

layout(std430, binding=3) buffer Color
{
    vec4 Colors[];
};

layout(std430, binding=4) buffer Lifetime
{
    vec2 Lifetimes[];
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

    gl_Position = vec4(Positions[id].xyz, 1.0);
    vLifetimePass = Lifetimes[id].x;
    vColorPass = Colors[id];
}