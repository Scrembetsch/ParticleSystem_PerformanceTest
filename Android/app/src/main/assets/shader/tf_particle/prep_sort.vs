#version VERSION

precision mediump float;

layout (location = 0) in vec2 aPosition;

out vec2 vTexCoord;

void main()
{
    gl_Position = vec4(aPosition, 0.0 , 1.0);
    vTexCoord = vec2(aPosition);
    vTexCoord = max(vTexCoord, 0.0);
}