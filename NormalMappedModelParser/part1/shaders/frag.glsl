#version 410 core

in vec2 v_TexCoord;
in vec3 v_FragPos;
in mat3 v_TBN;

uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalMap;

uniform vec3 u_LightPos;
uniform vec3 u_ViewPos;

out vec4 color;

void main()
{
    // Obtain normal from normal map
    vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(v_TBN * normal);

    // Lighting calculations
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular lighting
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // Combine results
    vec3 ambient = 0.1 * texture(u_DiffuseTexture, v_TexCoord).rgb;
    vec3 diffuse = diff * texture(u_DiffuseTexture, v_TexCoord).rgb;
    vec3 specular = spec * vec3(1.0); // White specular highlight

    vec3 finalColor = ambient + diffuse + specular;

    color = vec4(finalColor, 1.0);
}
