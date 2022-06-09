#version VERSION

precision mediump float;

DECL_TEX1
DECL_TEX4
DECL_TEX5

uniform mat4 uProjection;
uniform mat4 uView;

uniform float uCurrentTime;
uniform vec2 uResolution;

out vec2 vTexId;
out vec2 vTexCoord;

void main()
{
    uint id = uint(gl_VertexID / 6);
    uint subId = uint(gl_VertexID % 6);
    vec2 vId = vec2(0.0);
    vId.x = float(id % uint(uResolution.x));
    vId.y = float(id / uint(uResolution.x));
    vec2 offset = (1.0 / uResolution) / 2.0;
    vTexId = texture(USE_TEX5, (vId / uResolution) + offset).rg;

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

    position += vec3(-scale, -scale, 0.0) * bl;
    position += vec3(scale, -scale, 0.0) * br;
    position += vec3(-scale, scale, 0.0) * tl;
    position += vec3(scale, scale, 0.0) * tr;

    gl_Position = uProjection * uView * vec4(position, 1.0);
    vTexCoord = vec2(0.0, 0.0) * alive;
    vTexCoord += vec2(1.0, 0.0) * br * alive;
    vTexCoord += vec2(0.0, 1.0) * tl * alive;
    vTexCoord += vec2(1.0, 1.0) * tr * alive;
}