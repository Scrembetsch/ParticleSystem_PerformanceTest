#version VERSION

precision mediump float;

layout(std140, binding=4) buffer Position
{
    vec4 Positions[];
};

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    gl_Position = vec4(Positions[gl_VertexID].xyz, 1.0);
}