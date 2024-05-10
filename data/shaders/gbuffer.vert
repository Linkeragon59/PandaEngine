#version 450

layout (set = 0, binding = 0) uniform UBOScene
{
	mat4 projection;
	mat4 view;
} uboScene;

layout (set = 1, binding = 0) uniform UBOObject
{
    mat4 model;
} uboObject;

layout(std430, set = 1, binding = 3) readonly buffer JointMatrices
{
	mat4 jointMatrices[];
};

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;
layout (location = 4) in vec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

layout (location = 0) out vec3 outWorldPosition;
layout (location = 1) out vec3 outWorldNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	// Calculate skinned matrix from weights and joint indices of the current vertex
	mat4 skinMat = 
		inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
		inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
		inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
		inJointWeights.w * jointMatrices[int(inJointIndices.w)];
	mat4 skinnedModel = uboObject.model * skinMat;
	
	gl_Position = uboScene.projection * uboScene.view * skinnedModel * vec4(inPosition, 1.0);
	
	// Vertex position in world space
	outWorldPosition = vec3(skinnedModel * vec4(inPosition, 1.0));
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(skinnedModel)));
	outWorldNormal = mNormal * normalize(inNormal);	
	
	outUV = inUV;
	outColor = inColor;
}
