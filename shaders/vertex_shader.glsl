#version 330 core
layout (location = 0) in vec3 position;
uniform float scale;
uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uRotation;
uniform vec3 modelCenter;
uniform vec3 userOffset;

void main()
{
    vec3 centered = (position - modelCenter) * scale;
    vec3 rotated = (uRotation * vec4(centered, 1.0)).xyz; // w = 1.0 pq eh um ponto, nao um vetor
    vec3 finalPos = rotated + userOffset;

    gl_Position = uProjection * uView * vec4(finalPos, 1.0);
}