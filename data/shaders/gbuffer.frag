#version 450

layout (set = 1, binding = 1) uniform sampler2D samplerTexture;

layout (std430, set = 1, binding = 2) readonly buffer MaterialData
{
	vec4 color;
} materialData;

layout (location = 0) in vec3 inWorldPosition;
layout (location = 1) in vec3 inWorldNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

float linearDepth(float depth, float near, float far)
{
	return (near * far) / (far + depth * (near - far));	
}

void main() 
{
	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(0.0);
	
	// Store depth in alpha component
	outPosition = vec4(inWorldPosition, linearDepth(gl_FragCoord.z, 0.1, 256.0));
	
	outNormal = vec4(normalize(inWorldNormal), 1.0);
	
	vec4 sampledColor = texture(samplerTexture, inUV);
	outAlbedo = inColor * sampledColor * materialData.color;
}