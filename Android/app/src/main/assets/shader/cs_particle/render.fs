#version VERSION

precision mediump float;

DECL_TEX0

in vec2 vTexCoord;
in vec4 vColor;
out vec4 oColor;

void main()
{           
  vec4 vTexColor = texture(USE_TEX0, vTexCoord);
  oColor = vTexColor * vColor;
}