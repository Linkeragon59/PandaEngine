#version 450

layout (set = 1, binding = 0) uniform UBOScene
{
	mat4 projection;
	mat4 view;
} uboScene;

layout (set = 2, binding = 0) uniform UBOObject
{
    mat4 model;
} uboObject;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

void main () 
{
	gl_Position = uboScene.projection * uboScene.view * uboObject.model * vec4(inPosition, 1.0);
	
	outUV = inUV;
	outColor = inColor;	
}