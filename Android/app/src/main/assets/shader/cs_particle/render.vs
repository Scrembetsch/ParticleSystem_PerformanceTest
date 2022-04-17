#version VERSION

precision mediump float;

layout(std140, binding=1) buffer Position
{
    vec4 Positions[];
};

layout(std140, binding=3) buffer Color
{
    vec4 Colors[];
};

layout(std140, binding=4) buffer Lifetime
{
    vec4 Lifetimes[];
};

uniform mat4 uProjection;
uniform mat4 uView;

out float vLifetimePass;
out vec4 vColorPass;

void main()
{
    gl_Position = vec4(Positions[gl_VertexID].xyz, 1.0);
    vLifetimePass = Lifetimes[gl_VertexID].x;
    vColorPass = Colors[gl_VertexID];
    // vColorPass.r = 1.0 - Lifetimes[gl_VertexID].x / Lifetimes[gl_VertexID].y;
    // vColorPass.g = Lifetimes[gl_VertexID].x / Lifetimes[gl_VertexID].y;
    // vColorPass.b = 0.0;
    // vColorPass.a = 1.0;
    // vColorPass.a = Lifetimes[gl_VertexID].x / Lifetimes[gl_VertexID].y;
}