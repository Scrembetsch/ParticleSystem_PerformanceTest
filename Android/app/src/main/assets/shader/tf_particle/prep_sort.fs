#version VERSION

precision mediump float;

uniform vec2 uResolution;
uniform uint uNumAliveParticles;
in vec2 vTexCoord;

struct Particle
{
  vec4 Position;
  vec4 Velocity;
  vec4 Color;
  vec4 Data;
};

layout(binding = 1) uniform ParticleBuffer
{
  Particle Particles[MAX_PARTICLES];
};

out vec4 oIndex;

void main()
{
  oIndex.xy = vTexCoord;

  vec2 offset = (1.0 / uResolution) / 2.0;
  vec2 uid = (vTexCoord - offset) * uResolution;
  uint index = uint(floor(uid.x + uid.y * uResolution.x));

  if(Particles[index].Data.x > 0.0
    && index < uNumAliveParticles)
    oIndex.z = Particles[index].Position.w;
  else
    oIndex.z = -1.0;

    oIndex.a = 1.0;
}