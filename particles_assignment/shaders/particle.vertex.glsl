#version 330 core

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projMat;
uniform vec3 u_cameraRight;
uniform vec3 u_cameraUp;

layout(location = 0) in vec3 in_vert;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texCoord;

layout(location = 3) in vec3 in_offset;
layout(location = 4) in vec4 in_color;

out vec2 f_texCoord;
out vec4 f_color;
out vec3 f_worldPos;
out vec3 f_normal;

void main(void)
{
    vec3 vertexPosition = in_offset + 
        u_cameraRight * in_vert.x + 
        u_cameraUp * in_vert.y;

    vec4 worldPos = u_modelMat * vec4(vertexPosition, 1.0);
    gl_Position = u_projMat * u_viewMat * worldPos;
    
    f_texCoord = in_texCoord;
    f_color = in_color;
    f_worldPos = worldPos.xyz;
    f_normal = normalize(mat3(u_modelMat) * in_normal);
} 