#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aInstancePos;
layout (location = 4) in vec4 aInstanceColor;
layout (location = 5) in float aInstanceScale;

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 u_projMat;
uniform mat4 u_viewMat;
uniform mat4 u_modelMat;

void main()
{
    vec3 pos = aPos * aInstanceScale + aInstancePos;
    gl_Position = u_projMat * u_viewMat * u_modelMat * vec4(pos, 1.0);
    TexCoord = aTexCoord;
    ParticleColor = aInstanceColor;
} 