#version 330 core

uniform sampler2D u_particleTexture;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform float u_lightIntensity;

in vec2 f_texCoord;
in vec4 f_color;
in vec3 f_worldPos;
in vec3 f_normal;

out vec4 out_fragColor;

void main()
{
    // Sample the particle texture
    vec4 texColor = texture(u_particleTexture, f_texCoord);
    
    // Calculate lighting
    vec3 normal = normalize(f_normal);
    vec3 lightDir = normalize(u_lightPos - f_worldPos);
    vec3 viewDir = normalize(u_viewPos - f_worldPos);
    
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * u_lightColor;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * u_lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_lightColor;
    
    // Combine lighting with texture and particle color
    vec3 result = (ambient + diffuse + specular) * texColor.rgb * f_color.rgb;
    out_fragColor = vec4(result, texColor.a * f_color.a);
    
    // Make edges more transparent
    float edge = length(f_texCoord - vec2(0.5)) * 2.0;
    out_fragColor.a *= smoothstep(1.0, 0.0, edge);
    
    // Discard fully transparent fragments
    if(out_fragColor.a < 0.01)
        discard;
} 