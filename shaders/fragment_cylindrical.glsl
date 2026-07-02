#version 330 core

in vec3 vNormal;
in vec3 fragPosition;
in vec3 originalPosition;

out vec4 fragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform bool phong;
uniform vec2 x_lim;
uniform vec2 y_lim;

uniform sampler2D ourTexture;

void main()
{
    // Cylindrical texture projection
    float angle = atan(originalPosition.z, originalPosition.x);
    float u = angle / (2.0 * 3.14159265) + 0.5;
    float v = (originalPosition.y - y_lim.x) / (y_lim.y - y_lim.x);
    vec2 TexCoord = vec2(u, v);

    if (phong)
    {
        float ka = 0.5;
        vec3 ambient = ka * lightColor;

        float kd = 0.8;
        vec3 n = normalize(vNormal);
        vec3 l = normalize(lightPosition - fragPosition);

        float diff = max(dot(n,l), 0.0);
        vec3 diffuse = kd * diff * lightColor;

        float ks = 1.0;
        vec3 v = normalize(cameraPosition - fragPosition);
        vec3 r = reflect(-l, n);

        float spec = pow(max(dot(v, r), 0.0), 3.0);
        vec3 specular = ks * spec * lightColor;

        fragColor = texture(ourTexture, TexCoord) * vec4(ambient + diffuse + specular, 1.0);
    } else
    {
        fragColor = texture(ourTexture, TexCoord);
    }    
}
