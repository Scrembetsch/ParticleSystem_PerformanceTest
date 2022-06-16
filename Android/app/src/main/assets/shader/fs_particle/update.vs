#version VERSION

precision mediump float;

layout (location = 0) in vec2 aPosition;

uniform vec3 uRandomSeed;
uniform vec2 uResolution;

out vec2 vTexCoord;
out vec3 vRand;

void main()
{
    vRand = uRandomSeed * vec3((float(gl_VertexID) + 1.0) * 12.5, aPosition * 7.4);
    
    gl_Position = vec4(aPosition, 0.0 , 1.0);
    vTexCoord = vec2(aPosition);
    vec2 offset = (1.0 / uResolution) / 2.0;
    vTexCoord = max(vTexCoord, 0.0);
}