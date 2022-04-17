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

INDEX_BUFFER_DECL

uniform mat4 uProjection;
uniform mat4 uView;

out float vLifetimePass;
out vec4 vColorPass;

void main()
{
    INDEX_BUFFER_ID
    SORTED_VERTICES_ID
    
    gl_Position = vec4(Positions[id].xyz, 1.0);
    vLifetimePass = Lifetimes[id].x;
    vColorPass = Colors[id];
    // vColorPass.r = 1.0 - Lifetimes[gl_VertexID].x / Lifetimes[gl_VertexID].y;
    // vColorPass.g = Lifetimes[gl_VertexID].x / Lifetimes[gl_VertexID].y;
    // vColorPass.b = 0.0;
    // vColorPass.a = 1.0;
    // vColorPass.a = Lifetimes[gl_VertexID].x / Lifetimes[gl_VertexID].y;
}