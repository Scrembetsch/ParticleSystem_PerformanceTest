#version VERSION

precision mediump float;

layout(std140, binding=1) buffer Position
{
    vec3 Positions[];
};

layout(std140, binding=3) buffer Color
{
    vec4 Colors[];
};

layout(std140, binding=4) buffer Lifetime
{
    vec2 Lifetimes[];
};

uniform mat4 uProjection;
uniform mat4 uView;

out float vLifetimePass;

void main()
{
    gl_Position = vec4(Positions[gl_VertexID].xyz, 1.0);
    vLifetimePass = Lifetimes[gl_VertexID].x;
}