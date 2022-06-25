#version VERSION

precision mediump float;

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

uniform mat4 uProjection;
uniform mat4 uView;

uniform vec3 uQuad1;
uniform vec3 uQuad2;
uniform float uScale;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
   uint index = uint(gl_InstanceID);
   uint subId = uint(gl_VertexID);
   vec3 position = Particles[index].Position.xyz;

   float bl = float(subId == 0U);
   float br = float(subId == 1U);
   float tl = float(subId == 2U);
   float tr = float(subId == 3U);

   float alive = float(Particles[index].Data.x > 0.0 && Particles[index].Data.z == 0.0);
   float notAlive = float(Particles[index].Data.x <= 0.0 || Particles[index].Data.z > 0.0);

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

   vColor = Particles[index].Color;
}