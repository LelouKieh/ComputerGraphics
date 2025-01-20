#version 410 core
layout(location=0) in vec3 position;

// Uniform variables
uniform mat4 u_Projection; // perspective projection

void main()
{
    vec4 newPosition = u_Projection* vec4(position,1.0f);
    gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
}
