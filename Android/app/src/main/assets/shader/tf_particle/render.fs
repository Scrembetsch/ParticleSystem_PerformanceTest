#version VERSION

precision mediump float;

in vec2 vTexCoord;
in vec4 vColor;

DECL_TEX0

out vec4 outColor;

void main()
{
    outColor =  texture(USE_TEX0, vTexCoord) * vColor;
}