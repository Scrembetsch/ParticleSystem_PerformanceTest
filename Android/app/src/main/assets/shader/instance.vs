#version VERSION

layout(location = 0) in vec3 aBasePos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aPosition;
layout(location = 3) in vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

out vec4 vColor;
out vec2 vTexCoord;

void main()
{
    gl_Position = uProjection * uView * vec4(aBasePos + aPosition, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord;
}