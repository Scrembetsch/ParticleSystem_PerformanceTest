#version VERSION

layout(points) in;
layout(points) out;
layout(max_vertices = MAX_OUTPUT_VERTICES) out;

precision mediump float;

// All that we get from vertex shader

in vec3 vPositionPass[];
in vec3 vVelocityPass[];
in vec4 vColorPass[];
in float vLifeTimePass[];
in float vLifeTimeBeginPass[];
in float vTypePass[];
in uint vIdPass[];

// All that we send further
out vec3 vPositionOut;
out vec3 vVelocityOut;
out vec4 vColorOut;
out float vLifeTimeOut;
out float vLifeTimeBeginOut;
out float vTypeOut;

uniform vec3 uPosition; // Position where new particles are spawned
uniform vec3 uVelocityMin; // Velocity of new particle - from min to (min+range)
uniform vec3 uVelocityRange;

uniform float uLifeTimeMin;
uniform float uLifeTimeRange; // Life of new particle - from min to (min+range)
uniform float uTimeStep; // Time passed since last frame

uniform vec3 uRandomSeed; // Seed number for our random number function
vec3 lLocalSeed;

MODULE_UNIFORMS

// This function returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(lLocalSeed.y * 214013.0 + lLocalSeed.x * 2531011.0 + lLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    lLocalSeed = vec3(lLocalSeed.x + 147158.0 * fRes, lLocalSeed.y * fRes  + 415161.0 * fRes, lLocalSeed.z + 324154.0 * fRes);
    return fRes;
}

void InitParticle()
{
  vPositionOut = uPosition;
  vVelocityOut = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
  vLifeTimeOut = uLifeTimeMin + uLifeTimeRange * randZeroOne();
  vLifeTimeBeginOut = vLifeTimeOut;
  vColorOut = vec4(1.0);
  vTypeOut = 1.0;

  EmitVertex();
  EndPrimitive();
}

MODULE_METHODS

void main()
{
  lLocalSeed = vec3(uRandomSeed.x + gl_PrimitiveIDIn, uRandomSeed.y + gl_PrimitiveIDIn, uRandomSeed.z + gl_PrimitiveIDIn);
  
  vPositionOut = vPositionPass[0];
  vVelocityOut = vVelocityPass[0];
  vLifeTimeBeginOut = vLifeTimeBeginPass[0];
  vTypeOut = vTypePass[0];
  vColorOut = vColorPass[0];
  vLifeTimeOut = vLifeTimePass[0] - uTimeStep;

  if(vTypeOut != 0.0
    && vLifeTimeOut <= 0.0)
  {
    return;
  }

  MODULE_CALLS

  if(vTypePass[0] == 0.0)
  {
    return;
  }

  vPositionOut += vVelocityOut * uTimeStep;

  EmitVertex();
  EndPrimitive();
}