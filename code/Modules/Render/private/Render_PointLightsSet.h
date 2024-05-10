#pragma once

#include "Render_Buffer.h"

namespace Render
{
	struct PointLight
	{
		glm::vec4 myPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 myColor = glm::vec4(0.0f); // alpha channel used for intensity
	};

	class PointLightsSet
	{
	public:
		void Setup();
		void Destroy();

		void ClearLightData();
		void AddLight(const PointLight& aPointLight);
		void UpdateUBO();

		void Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter);

	private:
		static const uint ourMaxNumLights = 64;
		struct LightData
		{
			PointLight myLights[ourMaxNumLights];
		} myLightsData;
		uint myNumLights = 0;

		BufferPtr myLightsUBO;
	};
}
