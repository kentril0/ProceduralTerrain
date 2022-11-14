#version 450 core

layout(location = 0) in vec3 inTexCoord;

layout(location = 0) out vec4 outColor;

uniform samplerCube skybox;

void main()
{    
    outColor = texture(skybox, inTexCoord);
}

