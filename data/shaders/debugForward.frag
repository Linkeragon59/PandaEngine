#version 450

layout (set = 1, binding = 1) uniform sampler2D samplerTexture;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor * texture(samplerTexture, inUV);
}