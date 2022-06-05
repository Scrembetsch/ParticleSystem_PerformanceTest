#version VERSION

precision mediump float;

DECL_TEX1
DECL_TEX4

uniform mat4 uProjection;
uniform mat4 uView;

uniform float uCurrentTime;
uniform vec2 uResolution;

out vec2 vTexId;
out vec2 vTexCoord;

void main()
{
    uint id = uint(floor(gl_VertexID / 6.0));
    uint subId = uint(gl_VertexID % 6);
    vec2 vId = vec2(0.0);
    vId.x = float(id % uint(uResolution.x));
    vId.y = float(id / uint(uResolution.x));
    vTexId.x = (id % uint(uResolution.x) / uResolution.x);
    vTexId.y = (id / uint(uResolution.x) / uResolution.y);
    vec3 position = texture(USE_TEX1, vec2(vTexId.x, vTexId.y)).xyz;

    float bl = float(subId == 0);
    float br = float(subId == 1 || subId == 3);
    float tl = float(subId == 2 || subId == 4);
    float tr = float(subId == 5);

    float scale = 1.0;
    float deathTime = texture(USE_TEX4, vTexId).r;
    float aliveTime = texture(USE_TEX4, vTexId).g;

    float alive = float(uCurrentTime < deathTime);
    float notAlive = float(uCurrentTime >= deathTime);

    position += vec3(-scale, -scale, 0.0) * bl;
    position += vec3(scale, -scale, 0.0) * br;
    position += vec3(-scale, scale, 0.0) * tl;
    position += vec3(scale, scale, 0.0) * tr;

    gl_Position = uProjection * uView * vec4(position, 1.0);
    vTexCoord = vec2(0.0, 0.0);
    vTexCoord += vec2(1.0, 0.0) * br;
    vTexCoord += vec2(0.0, 1.0) * tl;
    vTexCoord += vec2(1.0, 1.0) * tr;
}