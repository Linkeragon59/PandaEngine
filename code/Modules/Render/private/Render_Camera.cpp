#include "Render_Camera.h"

#include "Render_ShaderHelpers.h"

namespace Render
{
	Camera::Camera()
	{
		myViewProjUBO = new Buffer(sizeof(ShaderHelpers::ViewProjMatricesData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myViewProjUBO->SetupDescriptor();
		myViewProjUBO->Map();

		Update(glm::mat4(1.0f), glm::mat4(1.0f));
	}

	void Camera::Update(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myView = aView;
		myProjection = aProjection;

		ShaderHelpers::ViewProjMatricesData viewProjData;
		viewProjData.myView = myView;
		viewProjData.myProjection = myProjection;
		memcpy(myViewProjUBO->myMappedData, &viewProjData, sizeof(ShaderHelpers::ViewProjMatricesData));
	}

	void Camera::Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter)
	{
		ShaderHelpers::CameraDescriptorInfo info;
		info.myViewProjMatricesInfo = &myViewProjUBO->myDescriptor;

		VkDescriptorSet cameraSet = myDescriptorSetGetter(info);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &cameraSet, 0, nullptr);
	}
}
