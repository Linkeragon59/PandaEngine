#pragma once

#include "Render_Renderer.h"

namespace Render
{
	// TODO : Rename SampleModelRenderer
	class SampleTexturedRectangleRenderer : public WorldRenderer
	{
	public:
		SampleTexturedRectangleRenderer(RenderTarget& aRenderTarget)
			: WorldRenderer(aRenderTarget)
		{}
	
	protected:
		void SetupInternal() override;
		void CleanupInternal() override;
	
		void StartFrameInternal() override;
		void EndFrameInternal() override;
	
	protected:
		void SetupDescriptorSets() override;
		void DestroyDescriptorSetLayouts() override;
		VkDescriptorSetLayout myCameraSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout mySimpleObjectSetLayout = VK_NULL_HANDLE;

		void SetupRenderPass() override;

		void SetupPipelines() override;
		void DestroyPipelines() override;
		VkPipeline myPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
	
	private:
		void SetupSimpleModel();
		Model* myModel = nullptr;
		Model* myModel2 = nullptr;
	};
}
