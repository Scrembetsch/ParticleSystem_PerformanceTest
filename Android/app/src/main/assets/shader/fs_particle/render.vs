#version VERSION

precision mediump float;

DECL_TEX1
DECL_TEX4
DECL_TEX5

uniform mat4 uProjection;
uniform mat4 uView;

uniform float uCurrentTime;
uniform float uScale;

uniform vec3 uQuad1;
uniform vec3 uQuad2;

uniform vec2 uResolution;

out vec2 vTexId;
out vec2 vTexCoord;

void main()
{
    uint id = uint(gl_VertexID) / 6U;
    uint subId = uint(gl_VertexID) % 6U;
    uvec2 uid = uvec2(0, 0);
    uid.x = id % uint(uResolution.x);
    uid.y = id / uint(uResolution.x);
    vTexId = texture(USE_TEX5, vec2(uid.xy / uResolution)).rg;

    vec3 position = texture(USE_TEX1, vTexId).xyz;

    float bl = float(subId == 0U);
    float br = float(subId == 1U || subId == 3U);
    float tl = float(subId == 2U || subId == 4U);
    float tr = float(subId == 5U);

    float scale = 0.5;
    float deathTime = texture(USE_TEX4, vTexId).r;
    float aliveTime = texture(USE_TEX4, vTexId).g;

    float alive = float(uCurrentTime < deathTime);
    float notAlive = float(uCurrentTime >= deathTime);

    position += (-uQuad1 - uQuad2) * uScale * bl;
    position += (-uQuad1 + uQuad2) * uScale * br;
    position += (uQuad1 - uQuad2) * uScale * tl;
    position += (uQuad1 + uQuad2) * uScale * tr;

    gl_Position = uProjection * uView * vec4(position, 1.0);
    vTexCoord = vec2(0.0, 0.0);
    vTexCoord += vec2(1.0, 0.0) * br;
    vTexCoord += vec2(0.0, 1.0) * tl;
    vTexCoord += vec2(1.0, 1.0) * tr;
}