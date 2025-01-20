#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // Obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, TexCoords).rgb;
    // Transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);
    // Transform normal vector to world space
    normal = normalize(TBN * normal);

    // Lighting calculations
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = diff * texture(diffuseMap, TexCoords).rgb;

    FragColor = vec4(diffuse, 1.0);
}
