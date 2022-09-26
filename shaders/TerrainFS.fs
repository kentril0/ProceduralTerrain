#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

//layout(binding = 0) uniform sampler2D colorMap;

const int kMaxColorCount = 8;

uniform float uMinHeight;
uniform float uMaxHeight;

uniform int uColorCount;
uniform vec3 uColors[kMaxColorCount];
uniform float uStartHeights[kMaxColorCount];


float InverseLerp(float a, float b, float x) {
    return clamp( (x-a) / (b-a), 0.0, 1.0);
}

void main()
{
    //outColor = vec4(texture(colorMap, inTexCoord).xyz, 1.0);

    const float kHeightPercent = InverseLerp(uMinHeight, uMaxHeight, inPos.y);

    vec3 color = vec3(0); //uColors[0];

    for (int i = 0; i < uColorCount; ++i) 
    {
        float opacity = clamp( sign(kHeightPercent - uStartHeights[i]), 0.0, 1.0);
        color = color*(1.0-opacity) + uColors[i]*opacity;
    }

    outColor = vec4(color, 1.0);
}
