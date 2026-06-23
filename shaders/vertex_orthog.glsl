#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
uniform float scale;
uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uRotation;
uniform mat4 model;
uniform vec3 modelCenter;
uniform vec3 userOffset;
uniform vec2 x_lim;
uniform vec2 y_lim;

out vec3 vNormal;
out vec3 fragPosition;
out vec2 TexCoord;

void main()
{
    vec3 centered = (position - modelCenter) * scale;
    vec3 rotated = (uRotation * vec4(centered, 1.0)).xyz;
    vec3 finalPos = rotated + userOffset;

    vec3 rotatedNormal = (uRotation * vec4(normal, 0.0)).xyz;
    vNormal = normalize(rotatedNormal);
    fragPosition = vec3(model * vec4(finalPos, 1.0));

    gl_Position = uProjection * uView * model * vec4(finalPos, 1.0);

    // Normalizar a textura para ir pra uv
    float u = (position.x - x_lim.x) / (x_lim.y - x_lim.x);
    float v = (position.y - y_lim.x) / (y_lim.y - y_lim.x);

    TexCoord = vec2(u, v);
}