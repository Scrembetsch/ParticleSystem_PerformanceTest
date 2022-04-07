#version VERSION

precision mediump float;

// Has to be here for mobile GL
in vec3 vPositionOut;
in vec3 vVelocityOut;
in vec4 vColorOut;
in float vLifeTimeOut;
in float vLifeTimeBeginOut;
in float vTypeOut;

out vec4 oColor;

void main()
{
  oColor = vec4(1.0);
}