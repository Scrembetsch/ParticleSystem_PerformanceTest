#version VERSION

layout(points) in;
layout(points) out;
layout(max_vertices = MAX_OUTPUT_VERTICES) out;

precision mediump float;

// All that we get from vertex shader

in vec4 vPositionPass[];
in vec4 vVelocityPass[];
in vec4 vColorPass[];
in vec4 vDataPass[];

// All that we send further
out vec4 vPositionOut;
out vec4 vVelocityOut;
out vec4 vColorOut;
out vec4 vDataOut;

uniform vec3 uPosition; // Position where new particles are spawned
uniform vec3 uCameraPos;
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
  vPositionOut = vec4(uPosition.xyz, distance(uPosition, uCameraPos));
  vVelocityOut = vec4(uVelocityMin + vec3(uVelocityRange.x * randZeroOne(), uVelocityRange.y * randZeroOne(), uVelocityRange.z * randZeroOne()), 0.0);
  vColorOut = vec4(1.0);
  vDataOut.x = uLifeTimeMin + uLifeTimeRange * randZeroOne();
  vDataOut.y = vDataOut.x;
  vDataOut.z = 0.0;

  EmitVertex();
  EndPrimitive();
}

MODULE_METHODS

void main()
{
  lLocalSeed = vec3(uRandomSeed.x + float(gl_PrimitiveIDIn), uRandomSeed.y + float(gl_PrimitiveIDIn), uRandomSeed.z + float(gl_PrimitiveIDIn));
  
  vPositionOut = vPositionPass[0];
  vVelocityOut = vVelocityPass[0];
  vColorOut = vColorPass[0];
  vDataOut = vDataPass[0];
  vDataOut.x -= uTimeStep;

  if(vDataOut.z == 0.0
    && vDataOut.x <= 0.0)
  {
    return;
  }

  MODULE_CALLS

  if(vDataPass[0].z != 0.0)
  {
    return;
  }

  vPositionOut.xyz += vVelocityOut.xyz * uTimeStep;
  vPositionOut.w = distance(vPositionOut.xyz, uCameraPos);

  EmitVertex();
  EndPrimitive();
}