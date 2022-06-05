#version VERSION

precision mediump float;

DECL_TEX1

uniform mat4 uProjection;
uniform mat4 uView;

uniform highp vec2 mResolution;

out vec2 vTexCoord;
out vec2 vId;

void main()
{
    uint id = uint(floor(gl_VertexID / 6.0));
    uint subId = uint(gl_VertexID % 6);
    vId.x = float(id % uint(mResolution.x));
    vId.y = float(id / uint(mResolution.x));
    gl_Position = vec4(texture(USE_TEX1, vec2(vId.x, vId.y)).rgb, 1.0);

    float bl = float(subId == 0);
    float br = float(subId == 1 || subId == 3);
    float tl = float(subId == 2 || subId == 4);
    float tr = float(subId == 5);

    float scale = 1.0;

    gl_Position += vec4(-scale, -scale, 0.0, 0.0) * bl;
    gl_Position += vec4(scale, -scale, 0.0, 0.0) * br;
    gl_Position += vec4(-scale, scale, 0.0, 0.0) * tl;
    gl_Position += vec4(scale, scale, 0.0, 0.0) * tr;

    vTexCoord = vec2(0.0, 0.0) * bl;
    vTexCoord = vec2(1.0, 0.0) * br;
    vTexCoord = vec2(0.0, 1.0) * tl;
    vTexCoord = vec2(1.0, 1.0) * tr;
}