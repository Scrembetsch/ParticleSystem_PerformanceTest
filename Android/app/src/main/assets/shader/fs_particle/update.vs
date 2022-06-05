#version VERSION

precision mediump float;

layout (location = 0) in vec2 aPosition;

uniform mat4 uProjection;
uniform mat4 uView;

out vec2 vTexCoord;

void main()
{
    gl_Position = vec4(aPosition, 0.0 , 1.0);
    vTexCoord = vec2(aPosition);
    vTexCoord.x = max(vTexCoord.x, 0.0);
    vTexCoord.y = max(vTexCoord.y, 0.0);
}