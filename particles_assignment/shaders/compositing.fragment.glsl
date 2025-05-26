#version 430 core

layout(binding = 0) uniform sampler2D u_diffuse;
layout(binding = 1) uniform sampler2D u_normal;
layout(binding = 2) uniform sampler2D u_position;
layout(binding = 3) uniform sampler2D u_shadowMap;
layout(binding = 4) uniform sampler2D u_ssao;

layout(location = 15) uniform vec3 u_lightPos;
layout(location = 20) uniform mat4 u_lightMat;
layout(location = 40) uniform mat4 u_lightProjMat;

const vec3 ambientColor = vec3(0.1, 0.1, 0.1);

in vec2 texCoords;

out vec4 fragColor;

void main() {
	vec3 position = texture(u_position, texCoords).xyz;
	vec3 normal = texture(u_normal, texCoords).xyz;

	vec3 diffuseColor = texture(u_diffuse, texCoords).xyz;
	float ambientOcclusion = texture(u_ssao, texCoords).r;

	// Debug visualization of SSAO
	// fragColor = vec4(vec3(ambientOcclusion), 1.0);
	// return;

	// Apply SSAO to ambient lighting
	vec3 ambient = vec3(0.3 * diffuseColor * ambientOcclusion);

	vec3 lightDir = normalize(u_lightPos - position);

	float lamb = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = lamb * diffuseColor;

	// Combine lighting with SSAO
	fragColor = vec4(diffuse + ambient, 1.0);

	vec4 shadowCoords = (u_lightProjMat * u_lightMat * vec4(position, 1.0));

	// shadowCoords are in light clipspace, but we get fragment relative 
	// coordinates in the shadowmap, so we need to remap to [0,1] interval in all dimensions
	vec3 mappedShadowCoords = (shadowCoords.xyz/shadowCoords.w) * 0.5 + 0.5;
	if (mappedShadowCoords.x > 0 && mappedShadowCoords.x < 1
		&& mappedShadowCoords.y > 0 && mappedShadowCoords.y < 1) {
		float shadow = texture(u_shadowMap, mappedShadowCoords.xy).x;
		if (shadow < (mappedShadowCoords.z - 0.000001)) {
			fragColor = vec4(0.5 * diffuseColor, 1.0);
		}
	}
}

