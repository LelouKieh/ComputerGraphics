#version 410 core

// Input attributes
layout(location = 0) in vec3 aPos;        // Position attribute
layout(location = 1) in vec2 aTexCoord;   // Texture coordinate attribute
layout(location = 2) in vec3 aNormal;     // Normal attribute (optional for future lighting)

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;

// Outputs to the fragment shader
out vec2 v_TexCoord;
// Optional outputs for lighting calculations
// out vec3 v_Normal;
// out vec3 v_FragPos;

void main()
{
    // Pass texture coordinates to the fragment shader
    v_TexCoord = aTexCoord;

    // Optionally pass transformed normals and fragment positions for lighting
    // mat3 normalMatrix = mat3(transpose(inverse(u_ModelMatrix)));
    // v_Normal = normalize(normalMatrix * aNormal);
    // v_FragPos = vec3(u_ModelMatrix * vec4(aPos, 1.0));

    // Compute the final position of the vertex
    gl_Position = u_Projection * u_ViewMatrix * u_ModelMatrix * vec4(aPos, 1.0);
}
