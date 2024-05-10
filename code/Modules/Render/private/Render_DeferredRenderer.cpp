#include "Render_DeferredRenderer.h"

#include "Render_Device.h"
#include "Render_Camera.h"
#include "Render_SwapChain.h"
#include "Render_ShaderHelpers.h"
#include "Render_Debug.h"
#include "Render_Model.h"
#include "Render_Gui.h"
#include "Render_ImGuiHelper.h"

namespace Render
{
	void DeferredRenderer::DrawGui(Gui* aGui)
	{
		aGui->Draw(myCurrentSecondaryCommandBufferGui, myGuiPipelineLayout, 0, [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet guiSet = GetFreeDescriptorSet(myGuiSetLayout);
			ShaderHelpers::UpdateDescriptorSet(guiSet, ShaderHelpers::BindingType::Gui, someInfo);
			return guiSet;
		});
	}

	void DeferredRenderer::DrawModel(Model* aModel)
	{
		aModel->Draw(myCurrentSecondaryCommandBufferGBuffer, myGBufferPipelineLayout, 1, [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet objectSet = GetFreeDescriptorSet(myObjectSetLayout);
			ShaderHelpers::UpdateDescriptorSet(objectSet, ShaderHelpers::BindingType::Object, someInfo);
			return objectSet;
		});
	}

	void DeferredRenderer::SetupInternal()
	{
		myPointLightsSet.Setup();
	}

	void DeferredRenderer::CleanupInternal()
	{
		myPointLightsSet.Destroy();
	}

	void DeferredRenderer::StartFrameInternal()
	{
		myPointLightsSet.ClearLightData();

		uint subPass = 0;
		myCurrentSecondaryCommandBufferGBuffer = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subPass++];
		myCurrentSecondaryCommandBufferCombine = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subPass++];
#if DEBUG_BUILD
		myCurrentSecondaryCommandBufferDebugForward = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subPass++];
#endif
		myCurrentSecondaryCommandBufferGui = myPerFrameData[myRenderTarget.GetCurrentImageIndex()].mySubPassesCommandBuffers[subPass++];

		vkCmdBindPipeline(myCurrentSecondaryCommandBufferGBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myGBufferPipeline);
		myCamera->Bind(myCurrentSecondaryCommandBufferGBuffer, myGBufferPipelineLayout, 0, [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet cameraSet = GetFreeDescriptorSet(myCameraSetLayout);
			ShaderHelpers::UpdateDescriptorSet(cameraSet, ShaderHelpers::BindingType::Camera, someInfo);
			return cameraSet;
		});

		vkCmdBindPipeline(myCurrentSecondaryCommandBufferCombine, VK_PIPELINE_BIND_POINT_GRAPHICS, myLightingPipeline);

#if DEBUG_BUILD
		vkCmdBindPipeline(myCurrentSecondaryCommandBufferDebugForward, VK_PIPELINE_BIND_POINT_GRAPHICS, myDebug3DPipeline);
		myCamera->Bind(myCurrentSecondaryCommandBufferDebugForward, myDebug3DPipelineLayout, 0, [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet cameraSet = GetFreeDescriptorSet(myCameraSetLayout);
			ShaderHelpers::UpdateDescriptorSet(cameraSet, ShaderHelpers::BindingType::Camera, someInfo);
			return cameraSet;
		});
#endif

		vkCmdBindPipeline(myCurrentSecondaryCommandBufferGui, VK_PIPELINE_BIND_POINT_GRAPHICS, myGuiPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myRenderTarget.GetExtent().width;
		viewport.height = (float)myRenderTarget.GetExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		SetViewport(viewport);

		VkRect2D scissor{};
		scissor.extent = myRenderTarget.GetExtent();
		scissor.offset = { 0, 0 };
		SetScissor(scissor);
	}

	void DeferredRenderer::EndFrameInternal()
	{
		myPointLightsSet.UpdateUBO();
		myPointLightsSet.Bind(myCurrentSecondaryCommandBufferCombine, myLightingPipelineLayout, 1, [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet lightsSet = GetFreeDescriptorSet(myLightsSetLayout);
			ShaderHelpers::UpdateDescriptorSet(lightsSet, ShaderHelpers::BindingType::LightsSet, someInfo);
			return lightsSet;
		});

		VkDescriptorSet compositionSet = GetFreeDescriptorSet(myCompositionSetLayout);

		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = compositionSet;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].pImageInfo = &myPositionAttachment.myDescriptor;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = compositionSet;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].pImageInfo = &myNormalAttachment.myDescriptor;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[2].dstSet = compositionSet;
		writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		writeDescriptorSets[2].dstBinding = 2;
		writeDescriptorSets[2].pImageInfo = &myAlbedoAttachment.myDescriptor;
		writeDescriptorSets[2].descriptorCount = 1;
		vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		vkCmdBindDescriptorSets(myCurrentSecondaryCommandBufferCombine, VK_PIPELINE_BIND_POINT_GRAPHICS, myLightingPipelineLayout, 0, 1, &compositionSet, 0, nullptr);

		vkCmdDraw(myCurrentSecondaryCommandBufferCombine, 4, 1, 0, 0);
	}

	std::string DeferredRenderer::GetSubpassDebugDescription(uint aSubpass) const
	{
		std::string description = WorldRenderer::GetSubpassDebugDescription(aSubpass);
		if (aSubpass == 0)
			description += ": GBuffer";
		else if (aSubpass == 1)
			description += ": Deferred Lighting";
#if DEBUG_BUILD
		else if (aSubpass == 2)
			description += ": Debug Forward";
#endif
		else
			description += ": Gui";
		return description;
	}

	void DeferredRenderer::SetupExtraAttachments()
	{
		VkClearValue attachmentClear{};
		attachmentClear.color = { 0.0f, 0.0f, 0.0f, 0.0f };

		myPositionAttachment.Create(myRenderTarget.GetExtent().width, myRenderTarget.GetExtent().height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myPositionAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myPositionAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myExtraAttachments.push_back(&myPositionAttachment);
		myExtraClearValues.push_back(attachmentClear);

		myNormalAttachment.Create(myRenderTarget.GetExtent().width, myRenderTarget.GetExtent().height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myNormalAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myNormalAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myExtraAttachments.push_back(&myNormalAttachment);
		myExtraClearValues.push_back(attachmentClear);

		myAlbedoAttachment.Create(myRenderTarget.GetExtent().width, myRenderTarget.GetExtent().height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myAlbedoAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myAlbedoAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myExtraAttachments.push_back(&myAlbedoAttachment);
		myExtraClearValues.push_back(attachmentClear);

		WorldRenderer::SetupExtraAttachments();
	}

	void DeferredRenderer::SetupDescriptorSets()
	{
		myCameraSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::Camera);
		mySimpleObjectSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::SimpleObject);
		myObjectSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::Object);
		myLightsSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::LightsSet);
		myGuiSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::Gui);

		// Composition Set Layout
		{
			std::array<VkDescriptorSetLayoutBinding, 3> bindings{};
			// Binding 0 : Position attachment
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Binding 1 : Normal attachment
			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Binding 2 : Albedo attachment
			bindings[2].binding = 2;
			bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[2].descriptorCount = 1;
			bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
			descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayoutInfo.bindingCount = (uint)bindings.size();
			descriptorLayoutInfo.pBindings = bindings.data();

			VK_CHECK_RESULT(
				vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myCompositionSetLayout),
				"Failed to create the Lighting descriptor set layout");
		}

#if DEBUG_BUILD
		uint nbCameraMax = 2;
#else
		uint nbCameraMax = 1;
#endif
		uint nbObjectsMax = 64;
		uint nbLightsMax = 16;
		uint nbGuiMax = 10;

		std::array<VkDescriptorPoolSize, 4> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = (nbCameraMax + nbObjectsMax + nbLightsMax) * myRenderTarget.GetImagesCount();
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = (nbObjectsMax + nbGuiMax) * myRenderTarget.GetImagesCount();
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = (2 * nbObjectsMax) * myRenderTarget.GetImagesCount();
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSizes[3].descriptorCount = (3 /*composition*/) * myRenderTarget.GetImagesCount();

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = (nbCameraMax + nbObjectsMax + 1 /*composition*/ + nbLightsMax + nbGuiMax) * myRenderTarget.GetImagesCount();

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void DeferredRenderer::DestroyDescriptorSetLayouts()
	{
		vkDestroyDescriptorSetLayout(myDevice, myCameraSetLayout, nullptr);
		myCameraSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(myDevice, mySimpleObjectSetLayout, nullptr);
		mySimpleObjectSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(myDevice, myObjectSetLayout, nullptr);
		myObjectSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(myDevice, myLightsSetLayout, nullptr);
		myLightsSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(myDevice, myGuiSetLayout, nullptr);
		myGuiSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(myDevice, myCompositionSetLayout, nullptr);
		myCompositionSetLayout = VK_NULL_HANDLE;
	}

	void DeferredRenderer::SetupRenderPass()
	{
		std::array<VkAttachmentDescription, 5> attachments{};
		{
			// Color attachment
			attachments[0].format = myRenderTarget.GetColorFormat();
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = myRenderTarget.GetFinalLayout();

			// Deferred attachments
			// Position
			attachments[1].format = myPositionAttachment.myFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Normals
			attachments[2].format = myNormalAttachment.myFormat;
			attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Albedo
			attachments[3].format = myAlbedoAttachment.myFormat;
			attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// Depth attachment
			attachments[4].format = myDepthFormat;
			attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		// Subpasses
		VkAttachmentReference colorReferences[4];
		colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[2] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[3] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentReference depthReference = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkAttachmentReference inputReferences[3];
		inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		std::vector<VkSubpassDescription> subpassDescriptions;
		{
			// First subpass: Fill G-Buffer components
			VkSubpassDescription gbufferDescription{};
			gbufferDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			gbufferDescription.colorAttachmentCount = 4;
			gbufferDescription.pColorAttachments = colorReferences;
			gbufferDescription.pDepthStencilAttachment = &depthReference;
			subpassDescriptions.push_back(gbufferDescription);

			// Second subpass: Final Lighting (using G-Buffer components)
			VkSubpassDescription lightingDescription{};
			lightingDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			lightingDescription.colorAttachmentCount = 1;
			lightingDescription.pColorAttachments = colorReferences;
			lightingDescription.pDepthStencilAttachment = &depthReference;
			lightingDescription.inputAttachmentCount = 3;
			lightingDescription.pInputAttachments = inputReferences;
			subpassDescriptions.push_back(lightingDescription);

#if DEBUG_BUILD
			// Third subpass: Debug Forward
			VkSubpassDescription debugForwardDescription{};
			debugForwardDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			debugForwardDescription.colorAttachmentCount = 1;
			debugForwardDescription.pColorAttachments = colorReferences;
			debugForwardDescription.pDepthStencilAttachment = &depthReference;
			debugForwardDescription.inputAttachmentCount = 1;
			debugForwardDescription.pInputAttachments = inputReferences;
			subpassDescriptions.push_back(debugForwardDescription);
#endif

			// Fourth subpass: Gui
			VkSubpassDescription guiDescription{};
			guiDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			guiDescription.colorAttachmentCount = 1;
			guiDescription.pColorAttachments = colorReferences;
			subpassDescriptions.push_back(guiDescription);
		}

		// TODO: Understand subpass dependencies better!
		std::vector<VkSubpassDependency> dependencies;
		{
			//uint srcSubpass = VK_SUBPASS_EXTERNAL;
			//uint dstSubpass = 0;
			//
			//VkSubpassDependency gbufferDependency{};
			//gbufferDependency.srcSubpass = srcSubpass;
			//gbufferDependency.dstSubpass = dstSubpass;
			//gbufferDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			//gbufferDependency.srcAccessMask = 0;
			//gbufferDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			//gbufferDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			//gbufferDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			//dependencies.push_back(gbufferDependency);
			//
			//srcSubpass = dstSubpass++;

			uint srcSubpass = 0;
			uint dstSubpass = 1;

			VkSubpassDependency lightingDependency{};
			lightingDependency.srcSubpass = srcSubpass;
			lightingDependency.dstSubpass = dstSubpass;
			lightingDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			lightingDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			lightingDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			lightingDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			lightingDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(lightingDependency);

			srcSubpass = dstSubpass++;

#if DEBUG_BUILD
			VkSubpassDependency debugForwardDependency{};
			debugForwardDependency.srcSubpass = srcSubpass;
			debugForwardDependency.dstSubpass = dstSubpass;
			debugForwardDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			debugForwardDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			debugForwardDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			debugForwardDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			debugForwardDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(debugForwardDependency);

			srcSubpass = dstSubpass++;
#endif

			VkSubpassDependency guiDependency{};
			guiDependency.srcSubpass = srcSubpass;
			guiDependency.dstSubpass = dstSubpass;
			guiDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			guiDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			guiDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			guiDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			guiDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(guiDependency);
		}

		// TODO : Use subpass dependencies for layout transitions

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<uint>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass), "Failed to create the render pass!");
	}

	void DeferredRenderer::SetupPipelines()
	{
		uint subpass = 0;
		SetupGBufferPipeline(subpass++);
		SetupLightingPipeline(subpass++);
#if DEBUG_BUILD
		SetupDebugForwardPipeline(subpass++);
#endif
		SetupGuiPipeline(subpass++);
	}

	void DeferredRenderer::DestroyPipelines()
	{
		vkDestroyPipeline(myDevice, myGBufferPipeline, nullptr);
		myGBufferPipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(myDevice, myGBufferPipelineLayout, nullptr);
		myGBufferPipelineLayout = VK_NULL_HANDLE;

		vkDestroyPipeline(myDevice, myLightingPipeline, nullptr);
		myLightingPipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(myDevice, myLightingPipelineLayout, nullptr);
		myLightingPipelineLayout = VK_NULL_HANDLE;

#if DEBUG_BUILD
		vkDestroyPipeline(myDevice, myDebug3DPipeline, nullptr);
		myDebug3DPipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(myDevice, myDebug3DPipelineLayout, nullptr);
		myDebug3DPipelineLayout = VK_NULL_HANDLE;
#endif

		vkDestroyPipeline(myDevice, myGuiPipeline, nullptr);
		myGuiPipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(myDevice, myGuiPipelineLayout, nullptr);
		myGuiPipelineLayout = VK_NULL_HANDLE;
	}

	void DeferredRenderer::SetViewport(const VkViewport& aViewport)
	{
		vkCmdSetViewport(myCurrentSecondaryCommandBufferGBuffer, 0, 1, &aViewport);
		vkCmdSetViewport(myCurrentSecondaryCommandBufferCombine, 0, 1, &aViewport);
#if DEBUG_BUILD
		vkCmdSetViewport(myCurrentSecondaryCommandBufferDebugForward, 0, 1, &aViewport);
#endif
		vkCmdSetViewport(myCurrentSecondaryCommandBufferGui, 0, 1, &aViewport);
	}

	void DeferredRenderer::SetScissor(const VkRect2D& aScissor)
	{
		vkCmdSetScissor(myCurrentSecondaryCommandBufferGBuffer, 0, 1, &aScissor);
		vkCmdSetScissor(myCurrentSecondaryCommandBufferCombine, 0, 1, &aScissor);
#if DEBUG_BUILD
		vkCmdSetScissor(myCurrentSecondaryCommandBufferDebugForward, 0, 1, &aScissor);
#endif
		vkCmdSetScissor(myCurrentSecondaryCommandBufferGui, 0, 1, &aScissor);
	}

	void DeferredRenderer::SetupGBufferPipeline(uint aSubpass)
	{
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
			myCameraSetLayout,
			myObjectSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myGBufferPipelineLayout),
			"Failed to create the GBuffer pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("shaders/gbuffer_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("shaders/gbuffer_frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pName = "main";

		auto bindingDescription = ShaderHelpers::Vertex::GetBindingDescription();
		auto attributeDescriptions = ShaderHelpers::Vertex::GetAttributeDescriptions({
			ShaderHelpers::VertexComponent::Position,
			ShaderHelpers::VertexComponent::Normal,
			ShaderHelpers::VertexComponent::UV,
			ShaderHelpers::VertexComponent::Color,
			ShaderHelpers::VertexComponent::Joint,
			ShaderHelpers::VertexComponent::Weight
		});

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint>(attributeDescriptions.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates{};
		blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[0].blendEnable = VK_FALSE;
		blendAttachmentStates[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[1].blendEnable = VK_FALSE;
		blendAttachmentStates[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[2].blendEnable = VK_FALSE;
		blendAttachmentStates[3].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[3].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = (uint)blendAttachmentStates.size();
		colorBlendState.pAttachments = blendAttachmentStates.data();

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myGBufferPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = aSubpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myGBufferPipeline),
			"Failed to create the GBuffer pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}

	void DeferredRenderer::SetupLightingPipeline(uint aSubpass)
	{
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
			myCompositionSetLayout,
			myLightsSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myLightingPipelineLayout),
			"Failed to create the Lighting pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("shaders/composition_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("shaders/composition_frag.spv");

		VkSpecializationMapEntry specializationEntry{};
		specializationEntry.constantID = 0;
		specializationEntry.offset = 0;
		specializationEntry.size = sizeof(uint32_t);

		uint32_t specializationData = 64;
		VkSpecializationInfo specializationInfo;
		specializationInfo.mapEntryCount = 1;
		specializationInfo.pMapEntries = &specializationEntry;
		specializationInfo.dataSize = sizeof(specializationData);
		specializationInfo.pData = &specializationData;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pSpecializationInfo = &specializationInfo;
		shaderStages[1].pName = "main";

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_NONE;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates{};
		blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = (uint)blendAttachmentStates.size();
		colorBlendState.pAttachments = blendAttachmentStates.data();

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myLightingPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = aSubpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myLightingPipeline),
			"Failed to create the Lighting pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}

#if DEBUG_BUILD
	void DeferredRenderer::SetupDebugForwardPipeline(uint aSubpass)
	{
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
			myCameraSetLayout,
			mySimpleObjectSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myDebug3DPipelineLayout),
			"Failed to create the Transparent pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("shaders/debugForward_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("shaders/debugForward_frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pName = "main";

		auto bindingDescription = ShaderHelpers::Vertex::GetBindingDescription();
		auto attributeDescriptions = ShaderHelpers::Vertex::GetAttributeDescriptions({
			ShaderHelpers::VertexComponent::Position,
			ShaderHelpers::VertexComponent::UV,
			ShaderHelpers::VertexComponent::Color
			});

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint>(attributeDescriptions.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_NONE;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates{};
		//blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		//blendAttachmentStates[0].blendEnable = VK_TRUE;
		//blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		//blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		//blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
		//blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		//blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		//blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = (uint)blendAttachmentStates.size();
		colorBlendState.pAttachments = blendAttachmentStates.data();

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myDebug3DPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = aSubpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myDebug3DPipeline),
			"Failed to create the transparent pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}
#endif

	void DeferredRenderer::SetupGuiPipeline(uint aSubpass)
	{
		std::array<VkDescriptorSetLayout, 1> descriptorSetLayouts = {
			myGuiSetLayout
		};

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ShaderHelpers::GuiPushConstBlock);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myGuiPipelineLayout),
			"Failed to create the GBuffer pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("shaders/gui_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("shaders/gui_frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pName = "main";

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(ImDrawVert);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint>(attributeDescriptions.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_NONE;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // TODO : Check
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myGuiPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = aSubpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myGuiPipeline),
			"Failed to create the GBuffer pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}
}
