#version 450

layout (set = 0, input_attachment_index = 0, binding = 0) uniform subpassInput samplerPositionDepth;
layout (set = 0, input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout (set = 0, input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int numLights = 64;

struct Light
{
	vec4 position;
	vec4 color;
};

layout (set = 1, binding = 0) uniform UBO 
{
	Light lights[numLights];
} lightUBO;

void main() 
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(samplerPositionDepth).rgb;
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	// Ambient part
	vec3 fragcolor  = albedo.rgb * 0.15;
	
	for(int i = 0; i < numLights; ++i)
	{
		if (lightUBO.lights[i].color.w <= 0)
			continue;
		
		vec3 L = normalize(lightUBO.lights[i].position.xyz - fragPos);
		vec3 N = normalize(normal);
		float NdotL = max(0.0, dot(N, L));
		float attenuation = lightUBO.lights[i].color.w / (pow(length(L), 2.0) + 1.0);
		fragcolor += lightUBO.lights[i].color.xyz * albedo.rgb * NdotL * attenuation;
	}
	
	outColor = vec4(fragcolor, 1.0);
}