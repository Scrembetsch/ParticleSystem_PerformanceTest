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

    // NEW
    vec3 position = vec3(0.0);

    float bl = float(subId == 0U);
    float br = float(subId == 1U || subId == 3U);
    float tl = float(subId == 2U || subId == 4U);
    float tr = float(subId == 5U);

    float scale = 0.5;
    position += vec3(-0.5, -0.5, 0.0) * bl;
    position += vec3(0.5, -0.5, 0.0) * br;
    position += vec3(-0.5, 0.5, 0.0) * tl;
    position += vec3(0.5, 0.5, 0.0) * tr;

    gl_Position = vec4(position, 1.0);
    vTexCoord = vec2(0.0, 0.0);
    vTexCoord += vec2(1.0, 0.0) * br;
    vTexCoord += vec2(0.0, 1.0) * tl;
    vTexCoord += vec2(1.0, 1.0) * tr;

    // OLD
    // uvec2 uid = uvec2(0, 0);
    // uid.x = id % uint(uResolution.x);
    // uid.y = id / uint(uResolution.x);
    // vTexId = texture(USE_TEX5, vec2(uid.xy) / uResolution).rg;

    // vec3 position = texture(USE_TEX1, vTexId).xyz;

    // float bl = float(subId == 0U);
    // float br = float(subId == 1U || subId == 3U);
    // float tl = float(subId == 2U || subId == 4U);
    // float tr = float(subId == 5U);

    // float scale = 0.5;
    // float deathTime = texture(USE_TEX4, vTexId).r;
    // float aliveTime = texture(USE_TEX4, vTexId).g;

    // float alive = float(uCurrentTime < deathTime);
    // float notAlive = float(uCurrentTime >= deathTime);


    // position = vec3(0.0);
    // position += (-uQuad1 - uQuad2) * uScale * 30.0 * bl;
    // position += (-uQuad1 + uQuad2) * uScale * 30.0 * br;
    // position += (uQuad1 - uQuad2) * uScale * 30.0 * tl;
    // position += (uQuad1 + uQuad2) * uScale * 30.0 * tr;

    // gl_Position = uProjection * uView * vec4(position, 1.0);
    // vTexCoord = vec2(0.0, 0.0);
    // vTexCoord += vec2(1.0, 0.0) * br;
    // vTexCoord += vec2(0.0, 1.0) * tl;
    // vTexCoord += vec2(1.0, 1.0) * tr;
}