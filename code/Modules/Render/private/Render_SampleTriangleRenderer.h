#pragma once

#include "Render_Renderer.h"

namespace Render
{
	class SampleTriangleRenderer : public Renderer
	{
	public:
		SampleTriangleRenderer(RenderTarget& aRenderTarget)
			: Renderer(aRenderTarget)
		{}
	
	protected:
		void StartFrameInternal() override;
		void EndFrameInternal() override;
	
	protected:
		void SetupDescriptorSets() override {}
		void DestroyDescriptorSetLayouts() override {}

		void SetupRenderPass() override;

		void SetupPipelines() override;
		void DestroyPipelines() override;
		VkPipeline myPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
	};
}
