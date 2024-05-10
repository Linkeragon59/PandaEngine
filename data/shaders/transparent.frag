#version 450

layout (set = 0, input_attachment_index = 0, binding = 0) uniform subpassInput samplerPositionDepth;

layout (set = 2, binding = 1) uniform sampler2D samplerTexture;

layout (std430, set = 2, binding = 2) readonly buffer MaterialData {
	vec4 color;
} materialData;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

float linearDepth(float depth, float near, float far)
{
	return (near * far) / (far + depth * (near - far));	
}

void main () 
{
	// Sample depth from deferred depth buffer and discard if obscured
	float depth = subpassLoad(samplerPositionDepth).a;

	// Save the sampled texture color before discarding.
	// This is to avoid implicit derivatives in non-uniform control flow.
	vec4 sampledColor = texture(samplerTexture, inUV);
	if ((depth != 0.0) && (linearDepth(gl_FragCoord.z, 0.1, 256.0) > depth))
	{
		discard;
	};

	outColor = inColor * sampledColor * materialData.color;
}
