#version 450

//layout(location = 0) in vec3 fs_position;
//layout(location = 1) in vec3 fs_normal;
layout(location = 0) in vec2 fsTex;
//layout(location = 3) in vec4 fs_color;

layout(location = 0) out vec4 final_color;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    //vec3 diffuse = texture(tex1, fs_texture_coordinate).rgb * fs_color;

    final_color = texture(tex, fsTex);
}
