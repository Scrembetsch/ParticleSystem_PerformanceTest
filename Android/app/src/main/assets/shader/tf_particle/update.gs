#version VERSION

layout(points) in;
layout(points) out;
layout(max_vertices = 10) out;

precision mediump float;

// All that we get from vertex shader

in vec3 vPositionPass[];
in vec3 vVelocityPass[];
in vec3 vColorPass[];
in float vLifeTimePass[];
in float vSizePass[];
in float vTypePass[];

// All that we send further
out vec3 vPositionOut;
out vec3 vVelocityOut;
out vec3 vColorOut;
out float vLifeTimeOut;
out float vSizeOut;
out float vTypeOut;

uniform vec3 uPosition; // Position where new particles are spawned
uniform vec3 uGravity; // Gravity vector for particles - updates velocity of particles 
uniform vec3 uVelocityMin; // Velocity of new particle - from min to (min+range)
uniform vec3 uVelocityRange;

uniform vec3 uColor;
uniform float uSize; 

uniform float uLifeTimeMin;
uniform float uLifeTimeRange; // Life of new particle - from min to (min+range)
uniform float uTimeStep; // Time passed since last frame

uniform vec3 uRandomSeed; // Seed number for our random number function
vec3 lLocalSeed;

uniform int uNumToGenerate; // How many particles will be generated next time, if greater than zero, particles are generated

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

void main()
{
  lLocalSeed = uRandomSeed;
  
  vPositionOut = vPositionPass[0];
  vVelocityOut = vVelocityPass[0];
  if(vTypePass[0] != 0.0)
  {
    vPositionOut += vVelocityOut * uTimeStep;
  }
  if(vTypePass[0] != 0.0)
  {
    vVelocityOut += uGravity * uTimeStep;
  }

  vColorOut = vColorPass[0];
  vLifeTimeOut = vLifeTimePass[0] - uTimeStep;
  vSizeOut = vSizePass[0];
  vTypeOut = vTypePass[0];

  if(vTypeOut == 0.0)
  {
    EmitVertex();
    EndPrimitive();
    
    for(int i = 0; i < uNumToGenerate; i++)
    {
      vPositionOut = uPosition;
      vVelocityOut = uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne());
      vColorOut = uColor;
      vLifeTimeOut = uLifeTimeMin + uLifeTimeRange * randZeroOne();
      vSizeOut = uSize;
      vTypeOut = 1.0;
      EmitVertex();
      EndPrimitive();
    }
  }
  else if(vLifeTimeOut > 0.0)
  {
      EmitVertex();
      EndPrimitive(); 
  }
}