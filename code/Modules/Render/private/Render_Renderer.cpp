#include "Render_Renderer.h"

#include "Render_Device.h"
#include "Render_Camera.h"
#include "Render_Gui.h"
#include "Render_Debug.h"

#include <format>

namespace Render
{
	Renderer::Renderer(RenderTarget& aRenderTarget)
		: myRenderTarget(aRenderTarget)
	{
		myDevice = RenderCore::GetInstance()->GetDevice();
	}

	void Renderer::Setup()
	{
		myPerFrameData.resize(myRenderTarget.GetImagesCount());

		// No dependencies
		SetupSecondaryCommandBuffers();
		SetupExtraAttachments();
		SetupDescriptorSets();

		// Depends on attachments
		SetupRenderPass();

		// Depends on descriptor sets and render pass
		SetupPipelines();

		SetupInternal();

		myIsSetup = true;
	}

	void Renderer::Cleanup()
	{
		myIsSetup = false;

		CleanupInternal();

		DestroyFrameBuffers();

		DestroyPipelines();

		DestroyRenderPass();

		DestroyDescriptorSets();
		DestroyExtraAttachments();
		DestroySecondaryCommandBuffers();

		myPerFrameData.clear();
	}

	void Renderer::StartFrame()
	{
		// Recycle descriptor sets
		for (auto& sets : myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myDescriptorSets)
		{
			sets.second.myFirstAvailableSet = 0;
		}

		// Recreate frame buffers
		std::vector<VkImageView> attachments;
		attachments.push_back(myRenderTarget.GetCurrentRenderTarget());
		for (Image* extraAttachment : myExtraAttachments)
			attachments.push_back(extraAttachment->myImageView);

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = myRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = myRenderTarget.GetExtent().width;
		framebufferInfo.height = myRenderTarget.GetExtent().height;
		framebufferInfo.layers = 1;
		if (myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myFrameBuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(myDevice, myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myFrameBuffer, nullptr);
		}
		VK_CHECK_RESULT(vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myFrameBuffer), "Failed to create a framebuffer!");

		myCurrentCommandBuffer = RenderCore::GetInstance()->GetCurrentCommandBuffer();
		myCurrentSecondaryCommandBuffer = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[0];

		VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo{};
		cmdBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		cmdBufferInheritanceInfo.renderPass = myRenderPass;
		cmdBufferInheritanceInfo.framebuffer = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myFrameBuffer;

		VkCommandBufferBeginInfo secondaryCmdBufferBeginInfo{};
		secondaryCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		secondaryCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		secondaryCmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;

		for (uint subpass = 0; subpass < GetSubpassesCount(); ++subpass)
		{
			cmdBufferInheritanceInfo.subpass = subpass;
			VK_CHECK_RESULT(vkBeginCommandBuffer(myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subpass], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
			Debug::BeginRegion(myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subpass], GetSubpassDebugDescription(subpass).c_str(), GetSubpassDebugColor(subpass));
		}

		StartFrameInternal();
	}

	void Renderer::EndFrame()
	{
		EndFrameInternal();

		for (uint subpass = 0; subpass < GetSubpassesCount(); ++subpass)
		{
			Debug::EndRegion(myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subpass]);
			VK_CHECK_RESULT(vkEndCommandBuffer(myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subpass]), "Failed to end a command buffer");
		}

		std::vector<VkClearValue> clearValues;
		VkClearValue renderTargetClear{};
		renderTargetClear.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues.push_back(renderTargetClear);
		for (VkClearValue& extraClearValue : myExtraClearValues)
			clearValues.push_back(extraClearValue);

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = myRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = myRenderTarget.GetExtent();
		renderPassBeginInfo.clearValueCount = (uint)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myFrameBuffer;

		vkCmdBeginRenderPass(myCurrentCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		for (uint subpass = 0; subpass < GetSubpassesCount(); ++subpass)
		{
			vkCmdExecuteCommands(myCurrentCommandBuffer, 1, &myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subpass]);

			if (subpass + 1 < GetSubpassesCount())
				vkCmdNextSubpass(myCurrentCommandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}

		vkCmdEndRenderPass(myCurrentCommandBuffer);
	}

	std::string Renderer::GetSubpassDebugDescription(uint aSubpass) const
	{
		return std::format("Subpass {}", aSubpass);
	}

	glm::vec4 Renderer::GetSubpassDebugColor(uint aSubpass) const
	{
		(void)aSubpass;
		return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void Renderer::SetupSecondaryCommandBuffers()
	{
		for (PerFrameData& perFrameData : myPerFrameData)
		{
			perFrameData.mySubPassesCommandBuffers.resize(GetSubpassesCount());

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();
			allocInfo.commandBufferCount = GetSubpassesCount();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, perFrameData.mySubPassesCommandBuffers.data()), "Failed to create secondary command buffers!");
		}
	}

	void Renderer::DestroySecondaryCommandBuffers()
	{
		for (PerFrameData& perFrameData : myPerFrameData)
			perFrameData.mySubPassesCommandBuffers.clear();
	}

	void Renderer::DestroyExtraAttachments()
	{
		for (Image* attachement : myExtraAttachments)
			attachement->Destroy();
		myExtraAttachments.clear();
	}

	void Renderer::DestroyDescriptorSets()
	{
		for (PerFrameData& perFrameData : myPerFrameData)
			perFrameData.myDescriptorSets.clear();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;

		DestroyDescriptorSetLayouts();
	}

	VkDescriptorSet Renderer::GetFreeDescriptorSet(VkDescriptorSetLayout aLayout)
	{
		PerFrameData::DescriptorSets& descriptorSets = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].myDescriptorSets[aLayout];

		if (descriptorSets.myFirstAvailableSet >= (uint)descriptorSets.mySets.size())
		{
			descriptorSets.myFirstAvailableSet = (uint)descriptorSets.mySets.size();

			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
			descriptorSetAllocateInfo.descriptorSetCount = 1;
			descriptorSetAllocateInfo.pSetLayouts = &aLayout;

			// TODO : Handle VK_ERROR_FRAGMENTED_POOL and VK_ERROR_OUT_OF_POOL_MEMORY
			VkDescriptorSet newSet;
			VK_CHECK_RESULT(
				vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &newSet),
				"Failed to allocate a DescriptorSet");

			descriptorSets.mySets.push_back(newSet);
		}

		return descriptorSets.mySets[descriptorSets.myFirstAvailableSet++];
	}

	void Renderer::DestroyRenderPass()
	{
		vkDestroyRenderPass(myDevice, myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;
	}

	void Renderer::DestroyFrameBuffers()
	{
		for (PerFrameData& perFrameData : myPerFrameData)
		{
			vkDestroyFramebuffer(myDevice, perFrameData.myFrameBuffer, nullptr);
			perFrameData.myFrameBuffer = VK_NULL_HANDLE;
		}
	}

	WorldRenderer::WorldRenderer(RenderTarget& aRenderTarget)
		: Renderer(aRenderTarget)
	{
		myCamera = new Camera();
		myDepthFormat = RenderCore::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();
	}

	WorldRenderer::~WorldRenderer()
	{
		SafeDelete(myCamera);
	}

	void WorldRenderer::SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myCamera->Update(aView, aProjection);
	}

	void WorldRenderer::SetupExtraAttachments()
	{
		myDepthAttachment.Create(myRenderTarget.GetExtent().width, myRenderTarget.GetExtent().height,
			myDepthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkImageAspectFlags aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Image::DepthFormatHasStencilAspect(myDepthFormat))
			aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
		myDepthAttachment.CreateImageView(aspects);

		myExtraAttachments.push_back(&myDepthAttachment);

		VkClearValue depthClear{};
		depthClear.depthStencil = { 1.0f, 0 };
		myExtraClearValues.push_back(depthClear);
	}
}
