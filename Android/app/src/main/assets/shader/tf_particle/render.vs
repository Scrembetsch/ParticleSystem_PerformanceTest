#version VERSION

precision mediump float;

layout (location = 0) in vec3 aPosition;
layout (location = 2) in vec3 aColor;
layout (location = 3) in float aLifeTime;
layout (location = 4) in float aSize;
layout (location = 5) in float aType;

out vec3 vColorPass;
out float vLifeTimePass;
out float vSizePass;
out float vTypePass;

void main()
{
   gl_Position = vec4(aPosition, 1.0);
   vColorPass = aColor;
   vSizePass = aSize;
   vLifeTimePass = aLifeTime;
}