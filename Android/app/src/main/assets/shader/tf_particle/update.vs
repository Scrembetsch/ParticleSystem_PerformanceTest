#version VERSION

precision mediump float;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aVelocity;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aLifeTime;
layout (location = 4) in float aLifeTimeBegin;
layout (location = 5) in float aType;

out vec3 vPositionPass;
out vec3 vVelocityPass;
out vec4 vColorPass;
out float vLifeTimePass;
out float vLifeTimeBeginPass;
out float vTypePass;
out uint vIdPass;

MODULE_UNIFORMS

MODULE_METHODS

void main()
{
  vPositionPass = aPosition;
  vVelocityPass = aVelocity;
  vColorPass = aColor;
  vLifeTimePass = aLifeTime;
  vLifeTimeBeginPass = aLifeTimeBegin;
  vTypePass = aType;
  vIdPass = gl_VertexID;

  MODULE_CALLS
}