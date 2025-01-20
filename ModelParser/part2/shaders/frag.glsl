#version 410 core

in vec3 v_vertexColors;
in vec3 v_vertexNormals;

out vec4 color;

void main()
{
    color = vec4(v_vertexColors, 1.0f);
}