#version 450

layout(location = 0) in vec2 fsTex;
layout(location = 1) in vec2 fsOpacity;

layout(location = 0) out vec4 final_color;

uniform sampler2DArray multiTex;
uniform sampler2DArray opacityMap;
uniform int layers;

void main()
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < layers; ++i)
    {
        vec4 tex = texture(multiTex, vec3(fsTex, i));
        color += tex.rgb * texture(opacityMap, vec3(fsOpacity, i)).r; 
        ///color += texture(opacityMap, vec3(fsTex, i)).a; 
    }
    final_color = vec4(color, 1.0);
}
