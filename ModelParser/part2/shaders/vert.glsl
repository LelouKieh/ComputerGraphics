#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormals;
layout(location = 2) in vec3 vertexColors;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;

out vec3 v_vertexColors;
out vec3 v_vertexNormals;

void main()
{
    v_vertexColors = vertexColors;
    v_vertexNormals = vertexNormals;
    gl_Position = u_Projection * u_ViewMatrix * u_ModelMatrix * vec4(position, 1.0f);
}