#version VERSION

precision mediump float;

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aVelocity;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec4 aData;

out float vTypePass;
out vec4 vColorPass;

void main()
{
   gl_Position = vec4(aPosition.xyz, 1.0);
   vTypePass = aData.z;
   vColorPass = aColor;
}