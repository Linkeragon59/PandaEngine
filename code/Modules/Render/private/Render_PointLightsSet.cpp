#include "Render_PointLightsSet.h"

#include "Render_ShaderHelpers.h"

namespace Render
{
	void PointLightsSet::Setup()
	{
		myLightsUBO = new Buffer(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myLightsUBO->SetupDescriptor();
		myLightsUBO->Map();

		ClearLightData();
	}

	void PointLightsSet::Destroy()
	{
		myLightsUBO = nullptr;
	}

	void PointLightsSet::ClearLightData()
	{
		for (PointLight& light : myLightsData.myLights)
		{
			light.myPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			light.myColor = glm::vec4(0.0f);
		}
		myNumLights = 0;
	}

	void PointLightsSet::AddLight(const PointLight& aPointLight)
	{
		if (myNumLights >= ourMaxNumLights)
			return;

		myLightsData.myLights[myNumLights] = aPointLight;
		myNumLights++;
	}

	void PointLightsSet::UpdateUBO()
	{
		memcpy(myLightsUBO->myMappedData, &myLightsData, sizeof(LightData));
	}

	void PointLightsSet::Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter)
	{
		ShaderHelpers::LightsSetDescriptorInfo info;
		info.myLightsInfo = &myLightsUBO->myDescriptor;
		VkDescriptorSet descriptorSet = myDescriptorSetGetter(info);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, nullptr);
	}
}
