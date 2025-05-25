#version 460 core

in vec2 TexCoord;
in vec4 ParticleColor;

out vec4 FragColor;

uniform vec4 u_solidColor;

void main()
{
    FragColor = u_solidColor * ParticleColor;
    
    // Discard transparent pixels
    if(FragColor.a < 0.1)
        discard;
} 