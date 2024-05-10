#pragma once

#include "Render_Renderer.h"
#include "Render_Gui.h"

namespace Render
{
	class GuiRenderer : public Renderer
	{
	public:
		GuiRenderer(RenderTarget& aRenderTarget)
			: Renderer(aRenderTarget)
		{}

		bool ShouldDrawGui() const override { return true; }
		void DrawGui(Gui* aGui) override;
	
	protected:
		void StartFrameInternal() override;
		void EndFrameInternal() override;
	
	protected:
		void SetupDescriptorSets() override;
		void DestroyDescriptorSetLayouts() override;
		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;

		void SetupRenderPass() override;

		void SetupPipelines() override;
		void DestroyPipelines() override;
		VkPipeline myPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
	};
}
