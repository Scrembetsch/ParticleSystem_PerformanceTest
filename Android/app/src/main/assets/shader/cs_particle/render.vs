#version VERSION

precision mediump float;

struct BufferParticle
{
    vec3 Position;
    float Lifetime;
    vec3 Velocity;
    float BeginLifetime;
    vec4 Color;
};

struct IndexStruct
{
    uint Idx;
    float Distance;
};

layout(std430, binding=1) buffer Particle
{
    BufferParticle Particles[];
};

layout(std430, binding=2) buffer Index
{
    IndexStruct Indices[];
};

uniform mat4 uProjection;
uniform mat4 uView;

uniform vec3 uQuad1;
uniform vec3 uQuad2;
uniform float uScale;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
    uint id = Indices[gl_InstanceID].Idx;
    uint subId = uint(gl_VertexID);

    vec3 position = Particles[id].Position.xyz;

    float bl = float(subId == 0U);
    float br = float(subId == 1U);
    float tl = float(subId == 2U);
    float tr = float(subId == 3U);

    float alive = float(Particles[id].Lifetime > 0.0);
    float notAlive = float(Particles[id].Lifetime <= 0.0);

    position += (-uQuad1 - uQuad2) * uScale * bl;
    position += (-uQuad1 + uQuad2) * uScale * br;
    position += (uQuad1 - uQuad2) * uScale * tl;
    position += (uQuad1 + uQuad2) * uScale * tr;

    vec3 offset = vec3(99999999) * notAlive;
    gl_Position = uProjection * uView * vec4(position + offset, 1.0);
    vTexCoord = vec2(0.0, 0.0);
    vTexCoord += vec2(1.0, 0.0) * br;
    vTexCoord += vec2(0.0, 1.0) * tl;
    vTexCoord += vec2(1.0, 1.0) * tr;

    vColor = Particles[id].Color;
}