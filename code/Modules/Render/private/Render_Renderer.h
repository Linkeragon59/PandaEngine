#pragma once

#include "Render_SwapChain.h"

namespace Render
{
	class Camera;
	class Model;
	class Gui;

	class Renderer
	{
	public:
		Renderer(RenderTarget& aRenderTarget);
		virtual ~Renderer() = default;

		RenderTarget* GetRenderTarget() const { return &myRenderTarget; }

		virtual bool ShouldDrawGui() const { return false; }
		virtual void DrawGui(Gui* /*aGui*/) {}

		virtual bool ShouldDrawWorld() const { return false; }
		virtual void SetViewProj(const glm::mat4& /*aView*/, const glm::mat4& /*aProjection*/) {}
		virtual void DrawModel(Model* /*aModel*/) {}

		void Setup();
		void Cleanup();
		bool IsSetup() const { return myIsSetup; }

		void StartFrame();
		void EndFrame();

	protected:
		virtual void SetupInternal() {}
		virtual void CleanupInternal() {}

		virtual void StartFrameInternal() = 0;
		virtual void EndFrameInternal() = 0;

	protected:
		VkDevice myDevice = VK_NULL_HANDLE;
		RenderTarget& myRenderTarget;
		VkCommandBuffer myCurrentCommandBuffer = VK_NULL_HANDLE;

	protected:
		// Secondary Command Buffers
		virtual uint GetSubpassesCount() const { return 1; }
		virtual std::string GetSubpassDebugDescription(uint aSubpass) const;
		virtual glm::vec4 GetSubpassDebugColor(uint aSubpass) const;
		void SetupSecondaryCommandBuffers();
		void DestroySecondaryCommandBuffers();
		VkCommandBuffer myCurrentSecondaryCommandBuffer = VK_NULL_HANDLE;

		// Attachments
		virtual void SetupExtraAttachments() {}
		void DestroyExtraAttachments();
		std::vector<Image*> myExtraAttachments;
		std::vector<VkClearValue> myExtraClearValues;

		// Descriptor Sets
		virtual void SetupDescriptorSets() = 0;
		virtual void DestroyDescriptorSetLayouts() = 0;
		void DestroyDescriptorSets();
		VkDescriptorSet GetFreeDescriptorSet(VkDescriptorSetLayout aLayout);
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		// Render Pass
		virtual void SetupRenderPass() = 0;
		void DestroyRenderPass();
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		// Frame Buffers
		void DestroyFrameBuffers();

		// Pipeline
		virtual void SetupPipelines() = 0;
		virtual void DestroyPipelines() = 0;

		// Per frame data
		struct PerFrameData
		{
			std::vector<VkCommandBuffer> mySubPassesCommandBuffers;

			// TODO : Not sure if this should really be per frame...
			struct DescriptorSets
			{
				std::vector<VkDescriptorSet> mySets;
				uint myFirstAvailableSet = 0;
			};
			std::map<VkDescriptorSetLayout, DescriptorSets> myDescriptorSets;

			VkFramebuffer myFrameBuffer = VK_NULL_HANDLE;
		};
		std::vector<PerFrameData> myPerFrameData;

		bool myIsSetup = false;
	};

	class WorldRenderer : public Renderer
	{
	public:
		WorldRenderer(RenderTarget& aRenderTarget);
		~WorldRenderer() override;

		bool ShouldDrawWorld() const override { return true; }
		void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection) override;

	protected:
		// Attachments
		void SetupExtraAttachments() override;
		// Depth attachment
		Image myDepthAttachment;

		Camera* myCamera = nullptr;
		VkFormat myDepthFormat = VK_FORMAT_UNDEFINED;
	};
}
