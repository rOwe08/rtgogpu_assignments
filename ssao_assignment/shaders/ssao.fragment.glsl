#version 430 core
out vec4 FragColor;

in vec2 uv;

layout(binding = 0) uniform sampler2D u_position; 
layout(binding = 1) uniform sampler2D u_normal;  
layout(binding = 2) uniform sampler2D u_noise; 

uniform vec3 u_samples[64];

int kernelSize = 64;
uniform float u_radius;
uniform float u_bias;

uniform vec2 u_noiseScale;

uniform mat4 u_proj;
uniform mat4 u_view;

void main() {
    vec3 fragPos = texture(u_position, uv).xyz;
    vec3 normal = normalize(texture(u_normal, uv).xyz);
    
    if(length(fragPos) < 0.1) {
        FragColor = vec4(1.0);
        return;
    }

    vec4 viewPos = u_view * vec4(fragPos, 1.0);
    vec3 viewNormal = normalize(mat3(u_view) * normal);

    vec3 randomVec = normalize(texture(u_noise, uv * u_noiseScale).xyz);

    vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, viewNormal);

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i) {
        vec3 samplePos = viewPos.xyz + TBN * (u_samples[i] * u_radius);

        vec4 offset = u_proj * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        vec4 sampleViewPos = u_view * vec4(texture(u_position, offset.xy).xyz, 1.0);
        float sampleDepth = sampleViewPos.z;

        float rangeCheck = smoothstep(0.0, 1.0, u_radius / abs(viewPos.z - sampleDepth));
        occlusion += (sampleDepth <= samplePos.z + u_bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    float ao = 1.0 - (occlusion / kernelSize);
    float finalAo = pow(ao, 2.0); // Increased power for stronger effect
    
    FragColor = vec4(finalAo, finalAo, finalAo, 1.0);
}