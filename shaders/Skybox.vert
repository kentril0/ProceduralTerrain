#version 450 core

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outTexCoord;

uniform mat4 projview;

void main()
{
    outTexCoord = inPos;

    const vec4 kPos = projview * vec4(inPos, 1.0);
    gl_Position = kPos.xyww;
}  
