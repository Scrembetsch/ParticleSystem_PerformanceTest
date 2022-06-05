#version VERSION

precision mediump float;

DECL_TEX0
DECL_TEX3

in vec2 vTexCoord;
in vec2 vId;

out vec4 oColor;

void main()
{           
  oColor = texture(USE_TEX0, vTexCoord);
  oColor = oColor * texture(USE_TEX3, vId);
}