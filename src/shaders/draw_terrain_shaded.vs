#version 450

struct Light {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Object {
	mat4 model;
    mat3 normalMat;
	vec3 ambient;
	vec3 diffuse;
	vec4 specular;  // last component is shininess
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fsPosition;
layout(location = 1) out vec3 fsNormal;
layout(location = 2) out vec2 fsTexCoord;

uniform mat4 MVP;
uniform Object object;

void main()
{
    fsPosition = vec3(object.model * vec4(position, 1.0));
    fsNormal = object.normalMat * normal;
    fsTexCoord = texCoord;

    gl_Position = MVP * vec4(position, 1.0);
}

