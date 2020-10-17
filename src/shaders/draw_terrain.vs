#version 450



layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
//layout(location = 2) in vec2 texture_coordinate;
//layout(location = 3) in vec4 color;

//layout(location = 0) out vec3 fs_position;
//layout(location = 1) out vec3 fs_normal;
//layout(location = 2) out vec2 fs_texture_coordinate;
//layout(location = 3) out vec4 fs_color;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
	//fs_position = vec3(model * vec4(position, 1.0));
	//fs_normal = transpose(inverse(mat3(model))) * normal;
	//fs_texture_coordinate = texture_coordinate;
    //fs_color = color;

    gl_Position = proj * view * model * vec4(position, 1.0);
    //gl_Position = pos.xyww;
}

