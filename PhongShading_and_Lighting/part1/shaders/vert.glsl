#version 410 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexNormals;
layout(location=2) in vec3 vertexColors;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;

// Pass vertex colors into the fragment shader
out vec3 v_vertexNormals;
out vec3 v_worldSpaceFragment;
out vec3 v_vertexColors;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(u_ModelMatrix)));
    v_vertexNormals = normalize(normalMatrix * vertexNormals);
    
    // Send to fragment shader the normals and the vertex colors
    v_vertexColors = vertexColors;

    // Calculate in world space the position of the vertex
    vec4 worldPosition = u_ModelMatrix * vec4(position, 1.0);
    v_worldSpaceFragment = worldPosition.xyz;

    // Compute the MVP matrix
    gl_Position = u_Projection * u_ViewMatrix * u_ModelMatrix * vec4(position,1.0f);
}