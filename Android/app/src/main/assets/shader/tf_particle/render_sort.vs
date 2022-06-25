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

uniform mat4 uProjection;
uniform mat4 uView;

uniform vec3 uQuad1;
uniform vec3 uQuad2;
uniform float uScale;

out vec2 vTexCoord;
out vec4 vColor;

vec2 getIndexUV()
{
    vec2 offset = (1.0 / uResolution) / 2.0;

    uvec2 uid = uvec2(0, 0);
    uid.x = uint(gl_InstanceID) % uint(uResolution.x);
    uid.y = uint(gl_InstanceID) / uint(uResolution.x);

    return vec2(uid / uvec2(uResolution)) + offset;
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

    uint subId = uint(gl_VertexID);

    vec3 position = Particles[uIndex].Position.xyz;

    float bl = float(subId == 0U);
    float br = float(subId == 1U);
    float tl = float(subId == 2U);
    float tr = float(subId == 3U);

    float alive = float(Particles[uIndex].Data.x > 0.0);
    float notAlive = float(Particles[uIndex].Data.x <= 0.0);

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

    vColor = Particles[uIndex].Color;
}