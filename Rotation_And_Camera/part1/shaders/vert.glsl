#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexColors;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection; // a perspective projection

// Pass vertex colors into the fragment shader
out vec3 v_vertexColors;

void main()
{
  v_vertexColors = vertexColors;

  // Utilize MVP matrix
  vec4 newPosition =  vec4(position,1.0f);
	gl_Position = u_Projection * u_ViewMatrix * u_ModelMatrix * newPosition;
}
