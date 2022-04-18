#version VERSION

precision mediump float;

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aVelocity;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec4 aData;

out vec4 vPositionPass;
out vec4 vVelocityPass;
out vec4 vColorPass;
out vec4 vDataPass;

MODULE_UNIFORMS

MODULE_METHODS

void main()
{
  vPositionPass = aPosition;
  vVelocityPass = aVelocity;
  vColorPass = aColor;
  vDataPass = aData;

  MODULE_CALLS
}