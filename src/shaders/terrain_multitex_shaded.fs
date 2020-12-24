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
    vec4 specular;  // last component is shiniess
};

layout(location = 0) in vec3 fsPosition;
layout(location = 1) in vec3 fsNormal;
layout(location = 2) in vec2 fsTexCoord;
layout(location = 3) in vec2 fsOpacity;

layout(location = 0) out vec4 finalColor;

uniform int layers;

uniform Object object;
uniform vec3 viewPos;
uniform Light light;

uniform sampler2DArray multiTex;
uniform sampler2DArray opacityMap;

void main()
{

    vec3 ambient = light.ambient * object.ambient;

    // diffuse
    vec3 normal = normalize(fsNormal);
    // vec3 lightDir = normalize(light.position - fsPosition)
    vec3 lightDir = normalize(-light.direction);
    // Calc all texture gains
    vec3 color = vec3(0.0);
    for (int i = 0; i < layers; ++i)
    {
        color += texture(multiTex, vec3(fsTexCoord, i)).rgb * 
                 texture(opacityMap, vec3(fsOpacity, i)).r; 
    }
    vec3 diffuse = color * object.diffuse * light.diffuse * 
                   max(dot(normal, lightDir), 0.0);

    // specular
    vec3 viewDir = normalize(viewPos - fsPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 specular = light.specular * 
                    pow(max(dot(viewDir, reflectDir), 0.0), object.specular.w) *
                    object.specular.rgb;


    finalColor = vec4(ambient + diffuse + specular, 1.0);
}

