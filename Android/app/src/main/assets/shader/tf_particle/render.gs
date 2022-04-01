#version VERSION

precision mediump float;

uniform mat4 uProjection;
uniform mat4 uView;

uniform vec3 uQuad1;
uniform vec3 uQuad2;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vColorPass[];
in float vLifeTimePass[];
in float vSizePass[];
in float vTypePass[];

out vec2 vTexCoord;
out vec4 vColorPart;

void main()
{
  vec3 vPosOld = gl_in[0].gl_Position.xyz;
  float fSize = vSizePass[0];
  mat4 mVP = uProjection * uView;
  
  if(vLifeTimePass[0] > 3.0)
  {
    float normalized = 1.0 - ((vLifeTimePass[0] - 3.0) / 1.0);
    vColorPart = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), normalized);
  }
  else if(vLifeTimePass[0] > 1.0)
  {
    float normalized = 1.0 - ((vLifeTimePass[0] - 1.0) / 2.0);
    vColorPart = mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), normalized);
  }
  else
  {
    float normalized = 1.0 - vLifeTimePass[0];
    vColorPart = mix(vec4(0.0, 0.0, 1.0, 1.0), vec4(vColorPass[0], vLifeTimePass[0]), normalized);
  }
      
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