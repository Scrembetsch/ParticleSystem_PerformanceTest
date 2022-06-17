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

uniform vec3 uQuad1;
uniform vec3 uQuad2;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
    uint id = Indices[gl_InstanceID].Idx;
    // vId = id;
    uint subId = uint(gl_VertexID);

    vec3 position = Positions[id].xyz;

    float bl = float(subId == 0U);
    float br = float(subId == 1U);
    float tl = float(subId == 2U);
    float tr = float(subId == 3U);

    float scale = 0.5;

    float alive = float(Lifetimes[id].x > 0.0);
    float notAlive = float(Lifetimes[id].x <= 0.0);

    position += (-uQuad1 - uQuad2) * scale * bl;
    position += (-uQuad1 + uQuad2) * scale * br;
    position += (uQuad1 - uQuad2) * scale * tl;
    position += (uQuad1 + uQuad2) * scale * tr;

    vec3 offset = vec3(99999999) * notAlive;
    gl_Position = uProjection * uView * vec4(position + offset, 1.0);
    vTexCoord = vec2(0.0, 0.0);
    vTexCoord += vec2(1.0, 0.0) * br;
    vTexCoord += vec2(0.0, 1.0) * tl;
    vTexCoord += vec2(1.0, 1.0) * tr;

    vColor = Colors[id];
}