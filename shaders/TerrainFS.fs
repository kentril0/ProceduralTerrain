#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D colorMap;

void main()
{
    outColor = vec4(texture(colorMap, inTexCoord).xyz, 1.0);
}
