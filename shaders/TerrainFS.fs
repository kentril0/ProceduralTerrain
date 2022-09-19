#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec4 outColor;

//layout(binding = 0) uniform sampler2D baseTexture;

void main()
{
    //outColor = texture(baseTexture, inTexCoord);
    //outColor = vec4(inTexCoord, 0.0, 1.0);
    outColor = vec4(inColor, 1.0);
}
