#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(inPos, 1.0);
    outNormal = inNormal;
    outTexCoord = inTexCoord;
}
