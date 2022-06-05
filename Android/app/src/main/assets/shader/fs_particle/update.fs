#version VERSION

precision mediump float;

layout (location = 0) out vec4 oPosition;
layout (location = 1) out vec4 oVelocity;
layout (location = 2) out vec4 oColor;

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

MODULE_UNIFORMS

in vec2 vTexCoord;
in vec3 vRand;

vec3 lLocalSeed;

struct Particle
{
    vec3 Position;
    float DistanceToCamera;
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
    uint n = floatBitsToUint(lLocalSeed.y * 2.113 + lLocalSeed.x * 1.892 + lLocalSeed.z * 1.4234);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    lLocalSeed = vec3(lLocalSeed.x + 14158.0 * fRes, lLocalSeed.y * fRes  + 411.0 * fRes, lLocalSeed.z + 254.0 * fRes);
    return fRes;
}

void InitParticle()
{
  lParticle.Position = uPosition;
  lParticle.DistanceToCamera = distance(uCameraPos, lParticle.Position);
  lParticle.Velocity = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
  lParticle.Color = vec4(1.0);
}

void InitLocalParticle()
{
  lParticle.Position = texture(USE_TEX1, vTexCoord).xyz;
  lParticle.DistanceToCamera = distance(lParticle.Position, uCameraPos);

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
  oPosition.w = lParticle.DistanceToCamera;

  oVelocity.xyz = lParticle.Velocity;
  oVelocity.w = 1.0;

  oColor = lParticle.Color;
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