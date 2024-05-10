#pragma once

namespace Render::ShaderHelpers
{
	//-----------------------------------------------------
	// Vertex Input
	//-----------------------------------------------------
	enum class VertexComponent
	{
		Position,
		Normal,
		UV,
		Color,
		Joint,
		Weight,
		Tangent
	};

	struct Vertex
	{
		glm::vec3 myPosition = glm::vec3(0.0f);
		glm::vec3 myNormal = glm::vec3(0.0f);
		glm::vec2 myUV = glm::vec2(0.0f);
		glm::vec4 myColor = glm::vec4(1.0f);
		glm::vec4 myJoint = glm::vec4(0.0f);
		glm::vec4 myWeight = glm::vec4(0.25f);
		glm::vec4 myTangent = glm::vec4(0.0f);

		static VkVertexInputBindingDescription GetBindingDescription(uint aBinding = 0);
		static VkVertexInputAttributeDescription GetAttributeDescription(VertexComponent aComponent, uint aLocation = 0, uint aBinding = 0);
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const std::vector<VertexComponent> someComponents, uint aBinding = 0);
	};

	//-----------------------------------------------------
	// Descriptor Sets
	//-----------------------------------------------------
	struct DescriptorInfo
	{};

	// Cameras must provide the following info to get a DescriptorSet to bind to a pipeline
	struct CameraDescriptorInfo : public DescriptorInfo
	{
		const VkDescriptorBufferInfo* myViewProjMatricesInfo = nullptr;
	};

	// Objects must provide the following info to get a DescriptorSet to bind to a pipeline
	struct ObjectDescriptorInfo : public DescriptorInfo
	{
		const VkDescriptorBufferInfo* myModelMatrixInfo = nullptr;
		const VkDescriptorImageInfo* myImageSamplerInfo = nullptr;
		const VkDescriptorBufferInfo* myMaterialInfo = nullptr;
		const VkDescriptorBufferInfo* myJointMatricesInfo = nullptr;
	};

	// LightsSets must provide the following info to get a DescriptorSet to bind to a pipeline
	struct LightsSetDescriptorInfo : public DescriptorInfo
	{
		const VkDescriptorBufferInfo* myLightsInfo = nullptr;
	};

	// Guis must provide the following info to get a DescriptorSet to bind to a pipeline
	struct GuiDescriptorInfo : public DescriptorInfo
	{
		const VkDescriptorImageInfo* myFontSamplerInfo = nullptr;
	};

	struct ViewProjMatricesData
	{
		glm::mat4 myProjection = glm::mat4(1.0f);
		glm::mat4 myView = glm::mat4(1.0f);
	};

	struct ModelMatrixData
	{
		glm::mat4 myModel = glm::mat4(1.0f);
	};

	struct MaterialData
	{
		glm::vec4 myColorFactor = glm::vec4(1.0f);
	};

	struct JointMatrixData
	{
		glm::mat4 myJointsMatrix = glm::mat4(1.0f);
	};

	struct GuiPushConstBlock
	{
		glm::vec2 myScale = glm::vec2(1.0f);
		glm::vec2 myTranslate = glm::vec2(0.0f);
	};

	// Shader pipelines can choose from the following available bindings
	enum class BindingType
	{
		Camera,
		SimpleObject,
		Object,
		LightsSet,
		Gui,
		Count,
	};

	VkDescriptorSetLayout CreateDescriptorSetLayout(BindingType aBindingType);
	void UpdateDescriptorSet(VkDescriptorSet aDescriptorSet, BindingType aBindingType, const ShaderHelpers::DescriptorInfo& someDescriptorInfo);

	VkShaderModule CreateShaderModule(const std::string& aFilename);
}
