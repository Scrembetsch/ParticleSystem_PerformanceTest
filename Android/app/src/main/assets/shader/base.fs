#version VERSION

precision mediump float;

in vec4 vColor;
in vec2 vTexCoord;

DECL_TEX0

out vec4 outColor;

void main()
{
    outColor =  texture(USE_TEX0, vTexCoord) * vColor;
}