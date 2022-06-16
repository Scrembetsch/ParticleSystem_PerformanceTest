#version VERSION

precision mediump float;

layout (location = 0) out vec4 oPosition;
layout (location = 1) out vec4 oVelocity;
layout (location = 2) out vec4 oColor;
layout (location = 3) out vec3 oIndex;

DECL_TEX1
DECL_TEX2
DECL_TEX3
DECL_TEX4

uniform vec3 uPosition;
uniform vec3 uCameraPos;
uniform float uCurrentTime;
uniform float uDeltaTime;
uniform vec3 uVelocityMin;
uniform vec3 uVelocityRange;
uniform float uLifeTimeMin;
uniform float uLifeTimeRange;
uniform vec2 uResolution;

MODULE_UNIFORMS

in vec2 vTexCoord;
in vec3 vRand;

vec3 lLocalSeed;

struct Particle
{
    vec3 Position;
    vec3 Velocity;
    vec4 Color;
    float DeathTime;
    float BirthTime;
    float TimeT;
    bool Alive;
};

Particle lParticle;

MODULE_METHODS

float randZeroOne()
{
  float value = fract(sin(dot(vec2(lLocalSeed.x + lLocalSeed.y, lLocalSeed.z), vec2(12.9898, 78.233))) * 43758.5453);
  lLocalSeed = vec3(lLocalSeed.x * value, lLocalSeed.y + value, lLocalSeed.z + 9.12 + value);
  return value;
  // uint n = floatBitsToUint(lLocalSeed.y * 1.2342 + lLocalSeed.x * 1.9283490 + lLocalSeed.z * 1.1568678);
  // n = n * (n * n * 151u + 7821u);
  // n = (n >> 9u) | 0x3F800000u;

  // float fRes =  2.0 - uintBitsToFloat(n);
  // lLocalSeed = vec3(lLocalSeed.x + 14.0 * fRes, lLocalSeed.y * fRes  + 41.0 * fRes, lLocalSeed.z + 2.0 * fRes);
  // return fRes;
}

void InitParticle()
{
  lParticle.Position = uPosition;
  lParticle.Velocity = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
  lParticle.Color = vec4(1.0);
}

void InitLocalParticle()
{
  lParticle.Position = texture(USE_TEX1, vTexCoord).xyz;

  lParticle.Velocity = texture(USE_TEX2, vTexCoord).xyz;

  lParticle.Color = texture(USE_TEX3, vTexCoord);

  lParticle.DeathTime = texture(USE_TEX4, vTexCoord).r;
  lParticle.BirthTime = texture(USE_TEX4, vTexCoord).g;
  lParticle.TimeT = (lParticle.DeathTime - uCurrentTime) / (lParticle.DeathTime - lParticle.BirthTime) ;
  lParticle.Alive = lParticle.DeathTime > uCurrentTime;
}

void SetOutputValues()
{
  oPosition.xyz = lParticle.Position;
  oPosition.w = distance(lParticle.Position, uCameraPos) * float(lParticle.Alive) - float(!lParticle.Alive);

  oVelocity.xyz = lParticle.Velocity;
  oVelocity.w = 1.0;

  oColor = lParticle.Color;

  oIndex.xy = vTexCoord;
  oIndex.z = oPosition.w;
}

void main()
{
  lLocalSeed = vRand;

  InitLocalParticle();

  if(uCurrentTime == lParticle.BirthTime)
  {
    InitParticle();
  }

  MODULE_CALLS

  lParticle.Position += lParticle.Velocity * uDeltaTime;
  lParticle.Position += vec3(9999999) * float(!lParticle.Alive);

  SetOutputValues();
}