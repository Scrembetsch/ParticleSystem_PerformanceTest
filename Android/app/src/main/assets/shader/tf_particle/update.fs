#version VERSION

precision mediump float;

// Has to be here for mobile GL
in vec4 vPositionOut;
in vec4 vVelocityOut;
in vec4 vColorOut;
in vec4 vDataOut;

out vec4 oColor;

void main()
{
  oColor = vec4(1.0);
}