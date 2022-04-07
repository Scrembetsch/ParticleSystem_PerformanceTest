#version VERSION

precision mediump float;

uniform mat4 uProjection;
uniform mat4 uView;

uniform vec3 uQuad1;
uniform vec3 uQuad2;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float vTypePass[];
in vec4 vColorPass[];

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
  if(vTypePass[0] == 0.0)
  {
    return;
  }

  vec3 vPosOld = gl_in[0].gl_Position.xyz;
  float fSize = 0.5;
  mat4 mVP = uProjection * uView;
  vColor = vColorPass[0];

  vec3 vPos = vPosOld + (-uQuad1 - uQuad2) * fSize;
  vTexCoord = vec2(0.0, 0.0);
  gl_Position = mVP * vec4(vPos, 1.0);
  EmitVertex();
  
  vPos = vPosOld+(-uQuad1 + uQuad2) * fSize;
  vTexCoord = vec2(0.0, 1.0);
  gl_Position = mVP * vec4(vPos, 1.0);
  EmitVertex();
  
  vPos = vPosOld + (uQuad1 - uQuad2) * fSize;
  vTexCoord = vec2(1.0, 0.0);
  gl_Position = mVP * vec4(vPos, 1.0);
  EmitVertex();
  
  vPos = vPosOld + (uQuad1 + uQuad2) * fSize;
  vTexCoord = vec2(1.0, 1.0);
  gl_Position = mVP * vec4(vPos, 1.0);
  EmitVertex();
  EndPrimitive();
}