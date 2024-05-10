#pragma once

#include "Render_Buffer.h"

namespace Render
{
	class Camera
	{
	public:
		Camera();

		const glm::mat4& GetView() const { return myView; }
		const glm::mat4& GetProjection() const { return myProjection; }

		void Update(const glm::mat4& aView, const glm::mat4& aProjection);
		void Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter);

	private:
		glm::mat4 myView = glm::mat4(1.0f);
		glm::mat4 myProjection = glm::mat4(1.0f);
		BufferPtr myViewProjUBO;
	};
}
