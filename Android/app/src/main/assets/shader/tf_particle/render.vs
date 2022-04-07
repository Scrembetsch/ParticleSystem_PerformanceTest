#version VERSION

precision mediump float;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aVelocity;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aLifeTime;
layout (location = 4) in float aLifeTimeBegin;
layout (location = 5) in float aType;

out float vTypePass;
out vec4 vColorPass;

void main()
{
   gl_Position = vec4(aPosition, 1.0);
   vTypePass = aType;
   vColorPass = aColor;
}