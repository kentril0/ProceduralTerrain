#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

uniform mat4 MVP;

void main()
{
    const vec4 kWorldPos = MVP * vec4(inPos, 1.0);
    gl_Position = kWorldPos;

    // TODO model
    outPos = inPos.xyz;
    outNormal = inNormal;
    outTexCoord = inTexCoord;
}
