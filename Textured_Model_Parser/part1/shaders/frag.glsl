#version 410 core

in vec2 v_TexCoord;
// Optional inputs for lighting calculations
// in vec3 v_Normal;
// in vec3 v_FragPos;

uniform sampler2D u_DiffuseTexture;
// Optional uniforms for lighting
// uniform vec3 u_LightPos;
// uniform vec3 u_ViewPos;

out vec4 color;

void main()
{
    // Sample the texture using the interpolated texture coordinates
    vec3 diffuseColor = texture(u_DiffuseTexture, v_TexCoord).rgb;

    color = vec4(diffuseColor, 1.0);
}
