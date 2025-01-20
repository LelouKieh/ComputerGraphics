#version 410 core

// Uniform variables
uniform vec3 u_ViewPos;

// Material properties
uniform vec3 u_MaterialAmbient;
uniform vec3 u_MaterialDiffuse;
uniform vec3 u_MaterialSpecular;
uniform float u_MaterialShininess;

// Light properties
uniform vec3 u_LightPos;
uniform vec3 u_LightAmbient;
uniform vec3 u_LightDiffuse;
uniform vec3 u_LightSpecular;

// Attenuation factors
uniform float u_LightConstant;
uniform float u_LightLinear;
uniform float u_LightQuadratic;

in vec3 v_vertexNormals;
in vec3 v_worldSpaceFragment;
in vec3 v_vertexColors;

out vec4 color;

// Entry point of program
void main()
{
    vec3 normals = normalize(v_vertexNormals);
    vec3 lightDir = normalize(u_LightPos - v_worldSpaceFragment);
    vec3 viewDir = normalize(u_ViewPos - v_worldSpaceFragment);
    vec3 reflectDir = reflect(-lightDir, normals);

    // Ambient component
    vec3 ambient = u_LightAmbient * abs(normals);

    // Diffuse component
    float diff = max(dot(normals, lightDir), 0.0);
    vec3 diffuse = u_LightDiffuse * diff * abs(normals);

    // Specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_MaterialShininess);
    vec3 specular = u_LightSpecular * spec * u_MaterialSpecular;

    // Attenuation calculation
    float distance = length(u_LightPos - v_worldSpaceFragment);
    float attenuation = 1.0 / (u_LightConstant + u_LightLinear * distance + u_LightQuadratic * (distance * distance));
    
    // Apply attenuation to each component
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Final color
    vec3 Lighting = ambient + diffuse + specular;
    color = vec4(Lighting, 1.0);
}