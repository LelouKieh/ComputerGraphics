#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out mat3 v_TBN;

void main()
{
    // Transform position
    vec4 worldPos = u_ModelMatrix * vec4(aPos, 1.0);
    gl_Position = u_Projection * u_ViewMatrix * worldPos;

    // Pass texture coordinates
    v_TexCoord = aTexCoord;

    // Calculate fragment position in world space
    v_FragPos = vec3(worldPos);

    // Calculate TBN matrix
    mat3 normalMatrix = transpose(inverse(mat3(u_ModelMatrix)));
    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 B = normalize(normalMatrix * a_Bitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    v_TBN = mat3(T, B, N);
}
