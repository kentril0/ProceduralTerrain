#version 450

layout(location = 0) in vec2 fsTex;

layout(location = 0) out vec4 final_color;

uniform sampler2DArray multiTex;
uniform int layers;

void main()
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < layers; ++i)
    {
        //final_color += texture(tex, vec3(fsTex, max(0, min(layers-1, floor(i + 0.5)))));
        vec4 tex = texture(multiTex, vec3(fsTex, i));
        color += tex.rgb * tex.a; 
    }
    final_color = vec4(color, 1.0);
}
