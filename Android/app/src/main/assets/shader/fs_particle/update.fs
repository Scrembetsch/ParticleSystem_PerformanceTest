#version VERSION

precision mediump float;

layout (location = 0) out vec4 oPosition;
layout (location = 1) out vec4 oVelocity;
layout (location = 2) out vec4 oColor;

DECL_TEX1
DECL_TEX2
DECL_TEX3
DECL_TEX4

uniform vec3 uCameraPos;
uniform float uCurrentTime;
uniform float uDeltaTime;

in vec2 vTexCoord;

void InitParticle()
{
  oPosition.xyz = vec3(0.0f);
  oPosition.w = distance(uCameraPos, oPosition.xyz);
  oVelocity = vec4(1.0, 1.0, 0.0, 0.0);
  oColor = vec4(1.0);
}

void main()
{
  float deathTime = texture(USE_TEX4, vTexCoord).r;
  float aliveTime = texture(USE_TEX4, vTexCoord).g;

  if(uCurrentTime == aliveTime)
  {
    InitParticle();
    return;
  }

  vec4 velocity = texture(USE_TEX2, vTexCoord);

  oPosition.xyz = texture(USE_TEX1, vTexCoord).xyz + velocity.xyz * uDeltaTime;
  oPosition.w = distance(uCameraPos, oPosition.xyz);
  oVelocity = velocity;
  oColor = texture(USE_TEX3, vTexCoord);
}