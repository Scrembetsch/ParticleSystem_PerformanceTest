#version VERSION

precision mediump float;

struct Particle
{
    vec4 Position;
    vec4 Velocity;
    vec4 Color;
    vec4 Lifetime;
};


layout(std140, binding=1) buffer ParticleBuffer
{
    Particle Particles[];
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
    
    gl_Position = vec4(Particles[id].Position.xyz, 1.0);
    vLifetimePass = Particles[id].Position.x;
    vColorPass = Particles[id].Color;
}