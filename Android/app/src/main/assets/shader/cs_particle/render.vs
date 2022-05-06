#version VERSION

precision mediump float;

layout (location = 1) in vec4 aPosition;
layout (location = 2) in vec4 aVelocity;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec4 aLifetime;

INDEX_BUFFER_DECL

uniform mat4 uProjection;
uniform mat4 uView;

out float vLifetimePass;
out vec4 vColorPass;

void main()
{
    INDEX_BUFFER_ID
    SORTED_VERTICES_ID
    
    gl_Position = vec4(aPosition.xyz, 1.0);
    vLifetimePass = aLifetime.x;
    vColorPass = aColor;
}