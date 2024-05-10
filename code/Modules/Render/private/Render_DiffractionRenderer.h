#pragma once

#include "Render_Renderer.h"

namespace Render
{
	class DiffractionRenderer : public Renderer
	{
	public:
		DiffractionRenderer(RenderTarget& aRenderTarget)
			: Renderer(aRenderTarget)
		{}
	
	protected:
		void SetupInternal() override;
		void CleanupInternal() override;
	
		void StartFrameInternal() override;
		void EndFrameInternal() override;
	
	protected:
		// Descriptor Sets
		void SetupDescriptorSets() override;
		void DestroyDescriptorSetLayouts() override;
		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;
	
		void SetupRenderPass() override;

		void SetupPipelines() override;
		void DestroyPipelines() override;
		VkPipeline myPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
	
	private:
		// vertex data, UBO and texture passed to the shaders
		void SetupRenderData();
		void DestroyRenderData();
		enum class VertexComponent
		{
			Position,
			UV
		};
		struct Vertex
		{
			glm::vec2 myPosition = glm::vec3(0.0f);
			glm::vec2 myUV = glm::vec2(0.0f);
			static VkVertexInputBindingDescription GetBindingDescription(uint aBinding = 0);
			static VkVertexInputAttributeDescription GetAttributeDescription(VertexComponent aComponent, uint aLocation, uint aBinding);
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const std::vector<VertexComponent> someComponents, uint aBinding = 0);
		};
		BufferPtr myVertexBuffer;
		BufferPtr myIndexBuffer;
		uint myIndexCount = 0;
		BufferPtr myUBOObject;
		ImagePtr myTexture;
		uint keyCallback = UINT_MAX;
	};
}
