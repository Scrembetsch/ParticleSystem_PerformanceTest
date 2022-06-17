#version VERSION

precision mediump float;

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aVelocity;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec4 aData;

uniform mat4 uProjection;
uniform mat4 uView;

uniform vec3 uQuad1;
uniform vec3 uQuad2;

out vec2 vTexCoord;
out vec4 vColor;

out uint vVertId;
out uint vInstId;

void main()
{
   vVertId = gl_VertexID;
   vInstId = gl_InstanceID;

   uint subId = gl_VertexID;
   vec3 position = aPosition.xyz;

   float bl = float(subId == 0U);
   float br = float(subId == 1U);
   float tl = float(subId == 2U);
   float tr = float(subId == 3U);

   float scale = 0.5;

   float alive = float(aData.x > 0.0 && aData.z == 0);
   float notAlive = float(aData.x <= 0.0 || aData.z > 0);

   position += (-uQuad1 - uQuad2) * scale * bl;
   position += (-uQuad1 + uQuad2) * scale * br;
   position += (uQuad1 - uQuad2) * scale * tl;
   position += (uQuad1 + uQuad2) * scale * tr;

   vec3 offset = vec3(99999999) * notAlive;
   gl_Position = uProjection * uView * vec4(position + offset, 1.0);
   vTexCoord = vec2(0.0, 0.0);
   vTexCoord += vec2(1.0, 0.0) * br;
   vTexCoord += vec2(0.0, 1.0) * tl;
   vTexCoord += vec2(1.0, 1.0) * tr;

   vColor = aColor;
}