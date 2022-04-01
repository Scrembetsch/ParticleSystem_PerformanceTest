#version VERSION

precision mediump float;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aVelocity;
layout (location = 2) in vec3 aColor;
layout (location = 3) in float aLifeTime;
layout (location = 4) in float aSize;
layout (location = 5) in float aType;

out vec3 vPositionPass;
out vec3 vVelocityPass;
out vec3 vColorPass;
out float vLifeTimePass;
out float vSizePass;
out float vTypePass;

void main()
{
  vPositionPass = aPosition;
  vVelocityPass = aVelocity;
  vColorPass = aColor;
  vLifeTimePass = aLifeTime;
  vSizePass = aSize;
  vTypePass = aType;
}