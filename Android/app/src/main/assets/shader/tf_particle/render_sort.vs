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

DECL_TEX1
uniform vec2 uResolution;

out float vTypePass;
out vec4 vColorPass;
out vec2 vIndexUV;
out vec3 vIndex;
out uint vUIndex;

vec2 getIndexUV()
{
   vec2 offset = (1.0 / uResolution) / 2.0;

   uvec2 uid = uvec2(0, 0);
   uid.x = uint(gl_VertexID) % uint(uResolution.x);
   uid.y = uint(gl_VertexID) / uint(uResolution.x);

   return vec2(uid / uResolution) + offset;
}

uint convertIndex(vec2 index)
{
   vec2 offset = (1.0 / uResolution) / 2.0;
   vec2 id = (index - offset) * uResolution;

   return uint(floor(id.x + id.y * uResolution.x));
}

void main()
{
    vec2 indexUV = getIndexUV();
    vec3 index = texture(USE_TEX1, indexUV).rgb;
    uint uIndex = convertIndex(index.xy);
    vIndex = index;
    vIndexUV = indexUV;
    vUIndex = uIndex;

    gl_Position = vec4(Particles[uIndex].Position.xyz, 1.0);
    vTypePass = Particles[uIndex].Data.z;
    vColorPass = Particles[uIndex].Color;
}