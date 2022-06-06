#version VERSION

precision mediump float;

layout (location = 0) in vec2 aPosition;

uniform mat4 uProjection;
uniform mat4 uView;
uniform vec3 uRandomSeed;

out vec2 vTexCoord;
out vec3 vRand;

void main()
{
    vRand = uRandomSeed * vec3((float(gl_VertexID) + 1.0) * 12.5, aPosition * 7.4);
    gl_Position = vec4(aPosition, 0.0 , 1.0);
    vTexCoord = vec2(aPosition);
    vTexCoord.x = max(vTexCoord.x, 0.0);
    vTexCoord.y = max(vTexCoord.y, 0.0);
}