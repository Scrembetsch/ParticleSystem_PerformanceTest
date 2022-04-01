#version VERSION

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

out vec4 vColor;

void main()
{
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
    vColor = vec4(aColor, 1.0);
}