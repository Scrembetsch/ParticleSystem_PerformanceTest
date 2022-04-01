#version VERSION

precision mediump float;

// All that we send further
in vec3 vPositionOut;
in vec3 vVelocityOut;
in vec3 vColorOut;
in float vLifeTimeOut;
in float vSizeOut;
in float vTypeOut;

out vec4 oColor;

void main()
{
  oColor = vec4(1.0);
}