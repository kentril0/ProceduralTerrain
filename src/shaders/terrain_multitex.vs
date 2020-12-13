#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex;

layout(location = 0) out vec2 fsTex;
layout(location = 1) out vec2 fsOpacity;

uniform vec2 scale;
uniform mat4 MVP;

void main()
{
	fsTex = scale * tex;
	fsOpacity = tex;

    gl_Position = MVP * vec4(position, 1.0);
}

