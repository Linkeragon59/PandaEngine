#pragma once

#include "Render_Renderer.h"
#include "Render_Image.h"
#include "Render_PointLightsSet.h"

namespace Render
{
	class DeferredRenderer : public WorldRenderer
	{
	public:
		DeferredRenderer(RenderTarget& aRenderTarget)
			: WorldRenderer(aRenderTarget)
		{}

		bool ShouldDrawGui() const override { return true; }
		void DrawGui(Gui* aGui) override;

		void DrawModel(Model* aModel) override;

	protected:
		void SetupInternal() override;
		void CleanupInternal() override;

		void StartFrameInternal() override;
		void EndFrameInternal() override;

	protected:
		// Secondary Command Buffers
#if DEBUG_BUILD
		uint GetSubpassesCount() const override { return 4; }
#else
		uint GetSubpassesCount() const override { return 3; }
#endif
		std::string GetSubpassDebugDescription(uint aSubpass) const override;
		VkCommandBuffer myCurrentSecondaryCommandBufferGBuffer = VK_NULL_HANDLE;
		VkCommandBuffer myCurrentSecondaryCommandBufferCombine = VK_NULL_HANDLE;
#if DEBUG_BUILD
		VkCommandBuffer myCurrentSecondaryCommandBufferDebugForward = VK_NULL_HANDLE;
#endif
		VkCommandBuffer myCurrentSecondaryCommandBufferGui = VK_NULL_HANDLE;

		// Attachments
		void SetupExtraAttachments() override;
		// GBuffer attachments
		Image myPositionAttachment;
		Image myNormalAttachment;
		Image myAlbedoAttachment;

		// Descriptor Sets
		void SetupDescriptorSets() override;
		void DestroyDescriptorSetLayouts() override;
		VkDescriptorSetLayout myCameraSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout mySimpleObjectSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myObjectSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myCompositionSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myLightsSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myGuiSetLayout = VK_NULL_HANDLE;

		// Render Pass
		void SetupRenderPass() override;

		// Pipelines
		void SetupPipelines() override;
		void DestroyPipelines() override;
		VkPipeline myGBufferPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myGBufferPipelineLayout = VK_NULL_HANDLE;
		VkPipeline myLightingPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myLightingPipelineLayout = VK_NULL_HANDLE;
#if DEBUG_BUILD
		VkPipeline myDebug3DPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myDebug3DPipelineLayout = VK_NULL_HANDLE;
#endif
		VkPipeline myGuiPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myGuiPipelineLayout = VK_NULL_HANDLE;

	private:
		void SetViewport(const VkViewport& aViewport);
		void SetScissor(const VkRect2D& aScissor);

		void SetupGBufferPipeline(uint aSubpass);
		void SetupLightingPipeline(uint aSubpass);
#if DEBUG_BUILD
		void SetupDebugForwardPipeline(uint aSubpass);
#endif
		void SetupGuiPipeline(uint aSubpass);

		// Point Lights
		PointLightsSet myPointLightsSet;
	};
}
